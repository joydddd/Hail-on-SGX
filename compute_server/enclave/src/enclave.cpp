#include <iostream>
#include <thread>
#include <map>
#include <fstream>
#include <atomic>
#include <chrono>

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
                    std::cout << "Set key failed.\n";
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
    Covar gwas_y;
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
            Covar new_y(buffer_decrypt);
            client_y_size[client] = new_y.size();
            client_info_list[client].size = new_y.size();
            gwas_y.combine(new_y);
        }
        delete[] y_buffer;
        //std::cout << "Y value loaded" << std::endl;
    } 
    catch (ERROR_t& err) {
        std::cerr << "ERROR: fail to get correct y values" << err.msg << std::endl;
    }

    gwas = new GWAS(gwas_y, LogReg_type);
    
    char covl[ENCLAVE_READ_BUFFER_SIZE];
    getcovlist(covl);
    std::string covlist(covl);
    std::vector<std::string> covariants;
    split_delim(covlist.c_str(), covariants);
    try{
        char* cov_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
        for (auto& cov : covariants) {
            if (cov == "1") {
                Covar intercept(gwas_y.size());
                gwas->add_covariant(intercept);
                continue;
            }
            Covar cov_var;
            for (int client = 0; client < num_clients; ++client) {
                int cov_buffer_size = 0;
                while (!cov_buffer_size) {
                    getcov(&cov_buffer_size, client, cov.c_str(), cov_buffer);
                }
                ClientInfo& info = client_info_list[client];
                std::memset(buffer_decrypt, 0, ENCLAVE_READ_BUFFER_SIZE);
                aes_decrypt_data(info.aes_list.front().aes_context,
                                info.aes_list.front().aes_iv,
                                (const unsigned char*) cov_buffer,
                                cov_buffer_size, 
                                (unsigned char*) buffer_decrypt);
                Covar new_cov_var(buffer_decrypt);
                if (new_cov_var.size() != client_y_size[client]) {
                    throw ReadtsvERROR("covariant size mismatch from client: " +
                                    std::to_string(client) + " with cov: " + new_cov_var.name() + 
                                    " size expected: " + std::to_string(client_y_size[client]) + " got: " + std::to_string(new_cov_var.size()));
                }
                cov_var.combine(new_cov_var);
            }
            gwas->add_covariant(cov_var);
        }
        delete[] cov_buffer;
        //std::cout << "GWAS setup finished" << std::endl;
    }
    catch (ERROR_t& err) {
        std::cerr << "ERROR: fail to get correct covariant values: " << err.msg << std::endl;
    }

    int total_row_size = 0;
    for (auto size : client_y_size) total_row_size += size;
    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        // TODO: set buffer type accordingly
        buffer_list[thread_id] = new Buffer(gwas, total_row_size, (Row_T)analysis_type, num_clients, thread_id);
    }
    //std::cout << "Buffer initialized" << std::endl;

    /* set up encrypted size */
    int total_crypto_size = 0;
    for (int i = 0; i < num_clients; i++) {
        int size = 0;
        while(!size) {
            get_encrypted_x_size(&size, i);
        }
        //std::cout << "client " << i << " crypto size: " << size << std::endl;
        client_info_list[i].crypto_size = size;
        total_crypto_size += size;
    }
    // Add padding for Loci + Allele and list of clients
    total_crypto_size += MAX_LOCI_ALLELE_STR_SIZE + (num_clients * 2);

    int max_batch_lines = ENCLAVE_READ_BUFFER_SIZE / total_crypto_size;
    if (!max_batch_lines) {
        std::cerr << "Data is too long to fit into enclave read buffer" << std::endl;
        exit(1);
    }
    setmaxbatchlines(max_batch_lines);

    start_thread = true;
    start_thread_cv.notify_all();
    std::cout << "Starting Enclave: "  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "\n";
}

void log_regression(const int thread_id) {
    std::string output_string;
    std::string loci_string;
    std::string alleles_string;
    output_string.reserve(50);
    loci_string.reserve(50);
    alleles_string.reserve(20);

    std::mutex useless_lock;
    std::unique_lock<std::mutex> useless_lock_wrapper(useless_lock);
    // experimental - checking to see if spinning up threads adds a noticable amount of overhead... need +1 TCS in config
    while(!start_thread) {
        start_thread_cv.wait(useless_lock_wrapper);
    }
    std::vector<double> change(gwas->dim());
    std::vector<double> old_beta(gwas->dim());
    
    Buffer* buffer = buffer_list[thread_id];
    Batch* batch = nullptr;
    Log_row* row;
    /* process rows */
    while (true) {
        start_timer("get_batch()");
        if (!batch || batch->st != Batch::Working) batch = buffer->launch(client_info_list, thread_id);
        if (!batch) {
            buffer->clean_up();
            break;
        }
        stop_timer("get_batch()");
        // get the next row from input buffer
        
        start_timer("get_row()");
        try {
            if (!(row = (Log_row*)batch->get_row(buffer))) continue;
        } catch (ERROR_t& err) {
            std::cerr << "ERROR: " << err.msg << std::endl << std::flush;
            exit(0);
        }
        stop_timer("get_row()");
        // compute results
        loci_to_str(row->getloci(), loci_string);
        alleles_to_str(row->getalleles(), alleles_string);
        output_string += loci_string + " " + alleles_string;
        bool converge = true;
        start_timer("converge()");
        try {
            converge = row->fit(change, old_beta);
            output_string += " " + std::to_string(row->output_first_beta_element()) + " " + std::to_string(row->t_stat()) + " ";
            // wanted to use a ternary, but the compiler doesn't like it?
            if (converge) {
                output_string += "true";
            } else {
                 output_string += "false";
            }
            output_string += "\n";
        } catch (MathError& err) {
            output_string += "\tNA\tNA\tNA\n";
            // cerr << "MathError while fiting " << ss.str() << ": " << err.msg
            //      << std::endl;
            //ss << "\tNA\tNA\tNA" << std::endl;
        } catch (ERROR_t& err) {
            std::cerr << "ERROR " << err.msg << std::endl << std::flush;
            //ss << "\tNA\tNA\tNA" << std::endl;
            exit(1);
        }
        stop_timer("converge()");
        start_timer("batch_write()");
        batch->write(output_string);
        output_string.clear();
        stop_timer("batch_write()");
    }

}

void linear_regression(const int thread_id) {
    std::string output_string;
    std::string loci_string;
    std::string alleles_string;
    output_string.reserve(50);
    loci_string.reserve(50);
    alleles_string.reserve(20);
    // experimental - checking to see if spinning up threads adds a noticable
    // amount of overhead... need +1 TCS in config
    while (!start_thread) {
        // spin until ready to go!
    }

    Buffer* buffer = buffer_list[thread_id];
    Batch* batch = nullptr;
    Lin_row* row;

    // DEBUG: tmp output file
    // ofstream out_st("enc" + std::to_string(thread_id) + ".out");

    /* process rows */
    while (true) {
        // start_timer("get_batch()");
        if (!batch || batch->st != Batch::Working)
            batch = buffer->launch(client_info_list, thread_id);
        if (!batch) {
            buffer->clean_up();
            break;
        }

        try {
            if (!(row = (Lin_row*)batch->get_row(buffer))) continue;
        } catch (ERROR_t& err) {
            std::cerr << "ERROR: " << err.msg << std::endl << std::flush;
            exit(0);
        }
 
        //  compute results
        loci_to_str(row->getloci(), loci_string);
        alleles_to_str(row->getalleles(), alleles_string);
        output_string += loci_string + "\t" + alleles_string;
        try {
            row->fit();
            output_string += "\t" +
                             std::to_string(row->output_first_beta_element()) +
                             "\t" + std::to_string(row->t_stat()) + "\t";
            // wanted to use a ternary, but the compiler doesn't like it?
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
        batch->write(output_string);
        output_string.clear();
        // stop_timer("batch_write()");
    }
}