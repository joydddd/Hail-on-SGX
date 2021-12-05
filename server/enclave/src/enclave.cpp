#include <iostream>
#include <thread>
#include <map>
#include <fstream>

#include "buffer.h"
#include "crypto.h"

#ifdef ENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif
Buffer* buffer;
vector<ClientInfo> clientinfo;

void setup(){
    RSACrypto rsa = RSACrypto();
    if (rsa.m_initialized) {
        cout << "CRYPTO WORKING!\n";
    }
    setrsapubkey(rsa.get_pub_key());

    cout << "RSA Pub Key Set\n";

    // We should store num_client number of aes keys/iv/contexts.
    for (auto& client:clientinfo) {
        AESData aes;
        aes.aes_context = new mbedtls_aes_context();
        mbedtls_aes_init(aes.aes_context);
        client.aes = aes;
    }

    int num_clients = clientinfo.size();

    try {
        unsigned char enc_aes_key[256];
        unsigned char enc_aes_iv[256];
        size_t* aes_length = new size_t(AES_KEY_LENGTH);

        for (int client = 0; client < num_clients; ++client) {
            bool rt = false;
            while (!rt) {
                getaes(&rt, client, enc_aes_key, enc_aes_iv);
            }
            rsa.decrypt(enc_aes_key, 256,
                        (uint8_t*)&clientinfo[client].aes.aes_key, aes_length);
            rsa.decrypt(enc_aes_iv, 256, (uint8_t*)&clientinfo[client].aes.aes_iv,
                        aes_length);
            // Initialize AES context so that we can decrypt data coming into
            // the enclave.
            int ret = mbedtls_aes_setkey_dec(clientinfo[client].aes.aes_context,
                                             clientinfo[client].aes.aes_key,
                                             AES_KEY_LENGTH * 8);
            if (ret != 0) {
                cout << "Set key failed.\n";
                exit(0);
            }
        }
        delete aes_length;
        cout << "AES KEY and IV loaded" << endl;
    } catch (ERROR_t& err) {
        cerr << "ERROR: fail to get AES KEY " << err.msg << endl;
    }

    /* set up encrypted size */
    for (int i = 0; i < num_clients; i++){
        int size = 0;
        while(!size) {
            get_encrypted_x_size(&size, i);
        }
        cout << "client " << i << " crypto size: " << size << endl;
        clientinfo[i].crypto_size = size;
    }
}

#include "logistic_regression.h"

void log_regression(){
    cout << "Logistic Regression started" << endl;
    /* setup gwas */
    char clientl[ENCLAVE_READ_BUFFER_SIZE];

    getclientlist(clientl);
    vector<string> clients;
    string clientl_str(clientl);
    int num_clients = split_tab(clientl_str, clients);

    clientinfo.resize(num_clients);
    setup();

    cout << "enclave running on " << clients.size() << " clients: " << endl;
    cout << clientl << endl;

    map<string, int> client_size_map;
    char* buffer_decrypt = new char[ENCLAVE_READ_BUFFER_SIZE];
    char* y_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    Log_var gwas_y;
    try {
        int client_num = 0;
        for (auto& client : clients) {
            int y_buffer_size = 0;
            while (!y_buffer_size){
                gety(&y_buffer_size, client.c_str(), y_buffer);
            }
            aes_decrypt_data(clientinfo[client_num].aes.aes_context, 
                        clientinfo[client_num].aes.aes_iv, 
                        (const unsigned char*) y_buffer, 
                        y_buffer_size, 
                        (unsigned char*) buffer_decrypt);
            stringstream y_ss(buffer_decrypt);
            Log_var new_y;
            new_y.read(y_ss);
            client_size_map[client] = new_y.size();
            clientinfo[client_num].size = new_y.size();
            gwas_y.combine(new_y);
            client_num++;
        }
        delete[] y_buffer;
        cout << "Y value loaded" << endl;
    } 
    catch (ERROR_t& err) {
        cerr << "ERROR: fail to get correct y values" << err.msg << endl;
    }

    Log_gwas gwas(gwas_y);

    char covl[ENCLAVE_READ_BUFFER_SIZE];
    getcovlist(covl);
    string covlist(covl);
    vector<string> covariants;
    split_tab(covlist, covariants);

    try{
        char* cov_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
        for (auto& cov : covariants) {
            if (cov == "1") {
                Log_var intercept(gwas_y.size());
                gwas.add_covariant(intercept);
                continue;
            }
            Log_var cov_var;
            int client_num = 0;
            for (auto& client : clients) {
                int cov_buffer_size = 0;
                while (!cov_buffer_size) {
                    getcov(&cov_buffer_size, client.c_str(), cov.c_str(), cov_buffer);
                }
                aes_decrypt_data(clientinfo[client_num].aes.aes_context, 
                             clientinfo[client_num].aes.aes_iv, 
                             (const unsigned char*) cov_buffer, 
                             cov_buffer_size, 
                             (unsigned char*) buffer_decrypt);
                stringstream cov_ss(buffer_decrypt);
                Log_var new_cov_var;
                new_cov_var.read(cov_ss);
                if (new_cov_var.size() != client_size_map[client])
                    throw ReadtsvERROR("covariant size mismatch from client: " +
                                    client);
                cov_var.combine(new_cov_var);
                client_num++;
            }
            gwas.add_covariant(cov_var);
        }
        delete[] cov_buffer;
        cout << "GWAS setup finished" << endl;
    }
    catch (ERROR_t& err) {
        cerr << "ERROR: fail to get correct covariant values: " << err.msg << endl;
    }

    /* setup read buffer */
    buffer = new Buffer(client_size_map[clients[0]], LOG_t);
    Batch* batch = nullptr;
    // DEBUG: tmp output file
    ofstream out_st("enc.out");

    cout << "Buffer initialized" << endl;
    /* process rows */
    while (true) {
        if (!batch || batch->st != Batch::Working) batch = buffer->launch();
        if (!batch) {
            // out_st << "End of Output" << endl;
            break;
        }
        // get the next row from input buffer
        Log_row* row;
        try {
            if (!(row = (Log_row*)batch->get_row())) continue;
        } catch (ERROR_t& err) {
            cerr << "ERROR: " << err.msg << endl;
            continue;
        }

        // compute results
        ostringstream ss;
        ss << row->getloci() << "\t" << row->getallels();
        bool converge;
        try {
            converge = row->fit(&gwas);
            ss << "\t" << row->beta()[0] << "\t" << row->t_stat();
            ss << "\t" << (converge ? "true" : "false");
            ss << endl;
            // cout << ss.str();
        } catch (MathError& err) {
            cerr << "MathError while fiting " << ss.str() << ": " << err.msg
                 << endl;
            ss << "\tNA\tNA\tNA" << endl;
        } catch (ERROR_t& err) {
            cerr << "ERROR " << ss.str() << ": " << err.msg << endl;
            ss << "\tNA\tNA\tNA" << endl;
            exit(1);
        }
        // DEBUG: tmp output to file
        out_st << ss.str();
        batch->write(ss.str());
    }
    cout << "Logistic regression Finished! " << endl;
}