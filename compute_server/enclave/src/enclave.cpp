#include <iostream>
#include <thread>
#include <map>
#include <fstream>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <string.h>

#include "buffer.h"
#include "crypto.h"
#include "gwas.h"

#ifdef NON_OE
#include "enclave_glue.h"
#else
#include "gwas_t.h"
#endif

static std::vector<Buffer*> buffer_list;
static std::vector<ClientInfo> client_info_list;
static std::vector<int> client_y_size;
static int num_clients;
static GWAS* gwas;

std::condition_variable start_thread_cv;
static volatile bool start_thread = false;

void setup_enclave_encryption(const int num_threads) {
    RSACrypto rsa = RSACrypto();
    if (!rsa.m_initialized) {
        std::cerr << "ERROR: failed to initialized RSA key" << std::endl;
        exit(1); 
    }
    setrsapubkey(rsa.get_pub_key());

    std::cout << "RSA Pub Key Set" << std::endl;

    getclientnum(&num_clients);

    std::cout << "enclave running on " << num_clients << " clients" << std::endl;

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
                    std::cout << "Set key failed." << std::endl;
                    exit(0);
                }
            }
        }
        delete aes_length;
        std::cout << "AES KEY and IV loaded" << std::endl;
    } catch (ERROR_t& err) {
        std::cerr << "ERROR: fail to get AES KEY " << err.msg << std::endl;
    }
}

void setup_enclave_phenotypes(const int num_threads, const int analysis_type) {
    char* buffer_decrypt = new char[ENCLAVE_READ_BUFFER_SIZE];
    char* y_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    Covar* gwas_y = new Covar();
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
            Covar* new_y = new Covar(buffer_decrypt);
            client_y_size[client] = new_y->size();
            client_info_list[client].size = new_y->size();
            gwas_y->combine(new_y);
            delete new_y;
        }
        delete[] y_buffer;
    } 
    catch (ERROR_t& err) {
        std::cerr << "ERROR: fail to get correct y values" << err.msg << std::endl;
    }

    gwas = new GWAS(gwas_y, LogReg_type);

    std::cout << "Y value loaded" << std::endl;

    char covl[ENCLAVE_SMALL_BUFFER_SIZE];
    getcovlist(covl);
    std::string covlist(covl);
    std::vector<std::string> covariant_names;
    split_delim(covlist.c_str(), covariant_names);

    try{
        char* cov_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
        for (auto& cov : covariant_names) {
            if (cov == "1") {
                Covar* intercept = new Covar(gwas_y->size());
                gwas->add_covariant(intercept);
                continue;
            }
            Covar *cov_var = new Covar();
            for (int client = 0; client < num_clients; ++client) {
                int cov_buffer_size = 0;
                memset(buffer_decrypt, 0, ENCLAVE_READ_BUFFER_SIZE);
                while (!cov_buffer_size) {
                    getcov(&cov_buffer_size, client, cov.c_str(), cov_buffer);
                }
                ClientInfo& info = client_info_list[client];
                aes_decrypt_data(info.aes_list.front().aes_context,
                                info.aes_list.front().aes_iv,
                                (const unsigned char*) cov_buffer,
                                cov_buffer_size, 
                                (unsigned char*) buffer_decrypt);
                Covar *new_cov_var = new Covar(buffer_decrypt, gwas_y->size());
                if (new_cov_var->size() != client_y_size[client]) {
                    throw ReadtsvERROR("covariant size mismatch from client: " +
                                    std::to_string(client) + " with cov: " + new_cov_var->name() + 
                                    " size expected: " + std::to_string(client_y_size[client]) + " got: " + std::to_string(new_cov_var->size()));
                }
                cov_var->combine(new_cov_var);
                delete new_cov_var;
            }
            gwas->add_covariant(cov_var);
        }
        delete[] cov_buffer;
        delete[] buffer_decrypt;
    } catch (ERROR_t& err) {
        std::cerr << "ERROR: fail to get correct covariant values: " << err.msg << std::endl;
    } catch (const std::exception &e) { 
        std::cout << "Crash in cov setup with " << e.what() << std::endl;
    }

    std::cout << "Cov loaded" << std::endl;


    int total_row_size = 0;
    for (auto size : client_y_size) total_row_size += size;

    try {
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            // TODO: set buffer type accordingly
            buffer_list[thread_id] = new Buffer(gwas, total_row_size, (Row_T)analysis_type, num_clients, thread_id);
        }
    } catch (const std::exception &e) { 
        std::cout << "Crash in buffer malloc with " << e.what() << std::endl;
    }

    /* set up encrypted size */
    int total_crypto_size = 0;
    for (int i = 0; i < num_clients; i++) {
        int size = 0;
        while(!size) {
            get_encrypted_x_size(&size, i);
        }
        client_info_list[i].crypto_size = size;
        // Add 1 for the tab delimiter
        total_crypto_size += size + 1;
    }
    // Add padding for Loci + Allele and list of clients + 1 for new line at very end of sequence
    total_crypto_size += MAX_LOCI_ALLELE_STR_SIZE + (num_clients * 2) + 1;
    
    int max_batch_lines = ENCLAVE_READ_BUFFER_SIZE / total_crypto_size;
    if (!max_batch_lines) {
        std::cerr << "Data is too long to fit into enclave read buffer" << std::endl;
        exit(1);
    }
    setmaxbatchlines(max_batch_lines);

    start_thread = true;
    start_thread_cv.notify_all();

    std::cout << "Setup finished" << std::endl;
}

void regression(const int thread_id, EncAnalysis analysis_type) {
    std::string output_string;
    std::string loci_string;
    std::string alleles_string;
    output_string.reserve(50);
    loci_string.reserve(50);
    alleles_string.reserve(20);
    int i = 0;

    std::mutex useless_lock;
    std::unique_lock<std::mutex> useless_lock_wrapper(useless_lock);
    // experimental - checking to see if spinning up threads adds a noticable
    // amount of overhead... need +1 TCS in config
    while (!start_thread) {
        start_thread_cv.wait(useless_lock_wrapper);
    }
    std::vector<double> change(gwas->dim());
    std::vector<double> old_beta(gwas->dim());

    Buffer* buffer = buffer_list[thread_id];
    Batch* batch = nullptr;
    Row* row;

    // DEBUG: tmp output file
    // ofstream out_st("enc" + std::to_string(thread_id) + ".out");

    /* process rows */
    while (true) {
        //start_timer("get_batch()");
        if (!batch || batch->st != Batch::Working)
            batch = buffer->launch(client_info_list, thread_id);
        if (!batch) {
            buffer->clean_up();
            break;
        }
        //stop_timer("get_batch()");
        //start_timer("get_row()");
        try {
            switch(analysis_type) {
                case EncAnalysis::linear:
                    if (!(row = (Lin_row*)batch->get_row(buffer))) continue;
                    break;
                case EncAnalysis::logistic:
                    if (!(row = (Log_row*)batch->get_row(buffer))) continue;
                    break;
                default:
                    throw std::runtime_error("Invalid analysis type");
            }
            
        } catch (ERROR_t& err) {
            std::cerr << "ERROR: " << err.msg << std::endl << std::flush;
            exit(0);
        } catch (const std::exception &e) { 
            std::cout << "Crash in get_row with " << e.what() << std::endl;
            exit(0);
        }
        //stop_timer("get_row()");
        //  compute results
        loci_to_str(row->getloci(), loci_string);
        alleles_to_str(row->getalleles(), alleles_string);
        output_string += loci_string + "\t" + alleles_string;
        //start_timer("converge()");
        bool converge = false;
        //std::cout << i++ << std::endl;
        try {
            switch (analysis_type) {
                case EncAnalysis::linear:
                    row->fit();
                    break;
                case EncAnalysis::logistic:
                    converge = row->fit(change, old_beta);
                    break;
            }
            output_string += "\t" +
                             std::to_string(row->output_first_beta_element()) +
                             "\t" + std::to_string(row->t_stat()) + "\t";
            // wanted to use a ternary, but the compiler doesn't like it?
            if (converge) {
                output_string += "true";
            } else if (analysis_type == EncAnalysis::logistic) {
                 output_string += "false";
            }
            output_string += "\n";
        } catch (MathError& err) {
            output_string += "\tNA\tNA\tNA\n";
            // cerr << "MathError while fiting " << ss.str() << ": " << err.msg
            //      << std::endl;
            // ss << "\tNA\tNA\tNA" << std::endl;
        } catch (ERROR_t& err) {
            std::cerr << "ERROR " << err.msg << std::endl << std::flush;
            // ss << "\tNA\tNA\tNA" << std::endl;
            exit(1);
        }
        //stop_timer("converge()");
        //start_timer("batch_write()");
        batch->write(output_string);
        output_string.clear();
        //stop_timer("batch_write()");
    }
}