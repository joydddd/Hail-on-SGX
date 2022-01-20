#include <iostream>
#include <thread>
#include <map>
#include <fstream>
#include <atomic>

#include "buffer.h"
#include "crypto.h"
#include "logistic_regression.h"

#ifdef ENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif

static std::vector<Buffer*> buffer_list;
static std::vector<ClientInfo> client_info_list;
static std::vector<int> client_y_size;
static int num_clients;
static Log_gwas* gwas;
static volatile bool start_thread = false;

void setup_enclave(const int num_threads) {
    RSACrypto rsa = RSACrypto();
    setrsapubkey(rsa.get_pub_key());

    cout << "RSA Pub Key Set\n";

    getclientnum(&num_clients);

    cout << "enclave running on " << num_clients << " clients: " << endl;

    client_info_list.resize(num_clients);
    client_y_size.resize(num_clients);
    buffer_list.resize(num_threads);

    // We should store num_client number of aes keys/iv/contexts.
    for (ClientInfo& client: client_info_list) {
        client.aes_list.resize(num_threads);
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            AESData aes;
            aes.aes_context = new mbedtls_aes_context();
            mbedtls_aes_init(aes.aes_context);
            client.aes_list[thread_id] = aes;
        }
    }

    try {
        unsigned char enc_aes_key[256];
        unsigned char enc_aes_iv[256];
        size_t* aes_length = new size_t(AES_KEY_LENGTH);

        for (int client = 0; client < num_clients; ++client) {
            for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
                bool rt = false;
                while (!rt) {
                    getaes(&rt, client, thread_id, enc_aes_key, enc_aes_iv);
                }
                AESData& thread_aes_data = client_info_list[client].aes_list[thread_id];
                rsa.decrypt(enc_aes_key, 256,
                            (uint8_t*)&thread_aes_data.aes_key, aes_length);
                rsa.decrypt(enc_aes_iv, 256, 
                            (uint8_t*)&thread_aes_data.aes_iv, aes_length);
                // Initialize AES context so that we can decrypt data coming into
                // the enclave.
                int ret = mbedtls_aes_setkey_dec(thread_aes_data.aes_context,
                                                 thread_aes_data.aes_key,
                                                 AES_KEY_LENGTH * 8);
                if (ret != 0) {
                    cout << "Set key failed.\n";
                    exit(0);
                }
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
        client_info_list[i].crypto_size = size;
    }

    char* buffer_decrypt = new char[ENCLAVE_READ_BUFFER_SIZE];
    char* y_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    Log_var gwas_y;
    try {
        for (int client = 0; client < num_clients; ++client) {
            int y_buffer_size = 0;
            while (!y_buffer_size){
                gety(&y_buffer_size, client, y_buffer);
            }
            ClientInfo& info = client_info_list[client]; 
            aes_decrypt_data(info.aes_list.front().aes_context,
                             info.aes_list.front().aes_iv,
                             (const unsigned char*) y_buffer,
                             y_buffer_size, 
                             (unsigned char*) buffer_decrypt);
            stringstream y_ss(buffer_decrypt);
            Log_var new_y;
            new_y.read(y_ss);
            client_y_size[client] = new_y.size();
            client_info_list[client].size = new_y.size();
            gwas_y.combine(new_y);
        }
        delete[] y_buffer;
        cout << "Y value loaded" << endl;
    } 
    catch (ERROR_t& err) {
        cerr << "ERROR: fail to get correct y values" << err.msg << endl;
    }

    gwas = new Log_gwas(gwas_y);
    
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
                gwas->add_covariant(intercept);
                continue;
            }
            Log_var cov_var;
            for (int client = 0; client < num_clients; ++client) {
                int cov_buffer_size = 0;
                while (!cov_buffer_size) {
                    getcov(&cov_buffer_size, client, cov.c_str(), cov_buffer);
                }
                ClientInfo& info = client_info_list[client];
                aes_decrypt_data(info.aes_list.front().aes_context,
                                info.aes_list.front().aes_iv,
                                (const unsigned char*) cov_buffer,
                                cov_buffer_size, 
                                (unsigned char*) buffer_decrypt);
                stringstream cov_ss(buffer_decrypt);
                Log_var new_cov_var;
                new_cov_var.read(cov_ss);
                if (new_cov_var.size() != client_y_size[client])
                    throw ReadtsvERROR("covariant size mismatch from client: " +
                                    std::to_string(client));
                cov_var.combine(new_cov_var);
            }
            gwas->add_covariant(cov_var);
        }
        delete[] cov_buffer;
        cout << "GWAS setup finished" << endl;
    }
    catch (ERROR_t& err) {
        cerr << "ERROR: fail to get correct covariant values: " << err.msg << endl;
    }

    int total_row_size = 0;
    for (auto size : client_y_size) total_row_size += size;
    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        buffer_list[thread_id] = new Buffer(total_row_size, LOG_t);
    }
    std::cout << "Buffer initialized" << endl;

    std::cout << "Setup finished\n";
    start_thread = true;
}

void log_regression(const int thread_id) {

    // experimental - checking to see if spinning up threads adds a noticable amount of overhead... need +1 TCS in config
    while(!start_thread) {
        // spin until ready to go!
    }

    Buffer* buffer = buffer_list[thread_id];
    Batch* batch = nullptr;
    // DEBUG: tmp output file
    ofstream out_st("enc" + std::to_string(thread_id) + ".out");
    
    /* process rows */
    while (true) {
        if (!batch || batch->st != Batch::Working) batch = buffer->launch(client_info_list, thread_id);
        if (!batch) {
            // out_st << "End of Output" << endl;
            break;
        }
        // get the next row from input buffer
        Log_row* row;
        try {
            if (!(row = (Log_row*)batch->get_row(buffer))) continue;
        } catch (ERROR_t& err) {
            cerr << "ERROR: " << err.msg << endl << std::flush;
            exit(0);
            continue;
        }
        // compute results
        ostringstream ss;
        ss << row->getloci() << "\t" << row->getallels();
        bool converge;
        try {
            converge = row->fit(gwas);
            ss << "\t" << row->beta()[0] << "\t" << row->t_stat();
            ss << "\t" << (converge ? "true" : "false");
            ss << endl;
            // cout << ss.str();
        } catch (MathError& err) {
            // cerr << "MathError while fiting " << ss.str() << ": " << err.msg
            //      << endl;
            ss << "\tNA\tNA\tNA" << endl;
        } catch (ERROR_t& err) {
            cerr << "ERROR " << ss.str() << ": " << err.msg << endl << std::flush;
            ss << "\tNA\tNA\tNA" << endl;
            exit(1);
        }
        // DEBUG: tmp output to file
        out_st << ss.str();
        batch->write(ss.str());
    }
}