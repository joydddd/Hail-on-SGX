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
static std::vector<Covar*> covar_ptr_list;
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

void setup_enclave_phenotypes(const int num_threads, EncAnalysis analysis_type, ImputePolicy impute_policy) {
    gwas = new GWAS(analysis_type);
    char* buffer_decrypt = new char[ENCLAVE_READ_BUFFER_SIZE];
    char* phenotype_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    Covar* gwas_y = new Covar();

    // Read in covariants from each institution
    char covl[ENCLAVE_SMALL_BUFFER_SIZE];
    getcovlist(covl);
    std::string covlist(covl);
    std::vector<std::string> covariant_names;
    split_delim(covlist.c_str(), covariant_names);

    // Read in number of patients at each institution
    char num_patients_buffer[ENCLAVE_SMALL_BUFFER_SIZE];
    int total_row_size = 0;
    for (int client = 0; client < num_clients; ++client) {
        int num_patients_buffer_size = 0;
        while (!num_patients_buffer_size){
            get_num_patients(&num_patients_buffer_size, client, num_patients_buffer);
        }
        ClientInfo& info = client_info_list[client];
        memset(buffer_decrypt, 0, ENCLAVE_READ_BUFFER_SIZE);
        aes_decrypt_data(info.aes_list.front().aes_context,
                         info.aes_list.front().aes_iv,
                         (const unsigned char*) num_patients_buffer,
                         num_patients_buffer_size, 
                         (unsigned char*) buffer_decrypt);
        int client_num_patients = std::stoi(buffer_decrypt);
        client_y_size[client] = client_num_patients;
        client_info_list[client].size = client_num_patients;
        total_row_size += client_num_patients;
    }

    gwas_y->reserve(total_row_size);
    for (int _ = 0; _ < covariant_names.size(); ++_) {
        Covar* cov_ptr = new Covar();
        cov_ptr->reserve(total_row_size);
        covar_ptr_list.push_back(cov_ptr);
    }

    try {
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            // TODO: set buffer type accordingly
            buffer_list[thread_id] = new Buffer(total_row_size, (Row_T)analysis_type, num_clients, thread_id);
        }
    } catch (const std::exception &e) { 
        std::cout << "Crash in buffer malloc with " << e.what() << std::endl;
    }

    /* set up encrypted size and max batch line */
    int total_crypto_size = 0;
    for (int client = 0; client < num_clients; client++) {
        // Calculate compaction factor, ceil(plaintext size / 4) -> rounded up to nearest multiple of 16
        int compacted_size = (client_y_size[client] + 3) / 4;
        int compacted_remainder = compacted_size % 16;
        if (compacted_remainder) {
            compacted_size += 16 - compacted_remainder;
        }
       // compacted_size += 16;

        client_info_list[client].crypto_size = compacted_size + 1;

        // Add 2 for the tab delimiter and null terminating char
        total_crypto_size += compacted_size + 2;
    }
    // Add padding for Loci + Allele and list of clients + 1 for new line at very end of sequence
    total_crypto_size += MAX_LOCI_ALLELE_STR_SIZE + (num_clients * 2) + 1;

    int max_batch_lines = ENCLAVE_READ_BUFFER_SIZE / total_crypto_size;
    if (!max_batch_lines) {
        std::cerr << "Data is too long to fit into enclave read buffer" << std::endl;
        exit(1);
    }
    setmaxbatchlines(max_batch_lines);
    

    std::cout << "Init finished" << std::endl;

    // Read in y-vals from each institution
    try {
        for (int client = 0; client < num_clients; ++client) {
            int y_buffer_size = 0;
            while (!y_buffer_size){
                gety(&y_buffer_size, client, phenotype_buffer);
            }
            ClientInfo& info = client_info_list[client];
            memset(buffer_decrypt, 0, ENCLAVE_READ_BUFFER_SIZE);
            aes_decrypt_data(info.aes_list.front().aes_context,
                             info.aes_list.front().aes_iv,
                             (const unsigned char*) phenotype_buffer,
                             y_buffer_size, 
                             (unsigned char*) buffer_decrypt);
            int read_size = gwas_y->read(buffer_decrypt, client_y_size[client]);
            if (read_size != client_y_size[client]) {
                throw ReadtsvERROR("y val size mismatch from client: " +
                                    std::to_string(client) + " size expected: " + 
                                    std::to_string(client_y_size[client]) + " got: " + 
                                    std::to_string(read_size));
            }
        }
    }
    catch (ERROR_t& err) {
        std::cerr << "ERROR: fail to get correct y values " << err.msg << std::endl;
    }

    gwas->add_y(gwas_y);
    std::cout << "Y value loaded" << std::endl;
    std::cout << "Starting Enclave: "  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "\n";
    try{
        for (int i = 0; i < covariant_names.size(); ++i) {
            const std::string& cov_name = covariant_names[i];
            Covar *cov_var = covar_ptr_list[i];
            if (cov_name == "1") {
                cov_var->init_1_covar(total_row_size);
                gwas->add_covariant(cov_var);
                continue;
            }
            for (int client = 0; client < num_clients; ++client) {
                int cov_buffer_size = 0;
                while (!cov_buffer_size) {
                    getcov(&cov_buffer_size, client, cov_name.c_str(), phenotype_buffer);
                }
                ClientInfo& info = client_info_list[client];
                memset(buffer_decrypt, 0, ENCLAVE_READ_BUFFER_SIZE);
                aes_decrypt_data(info.aes_list.front().aes_context,
                                info.aes_list.front().aes_iv,
                                (const unsigned char*) phenotype_buffer,
                                cov_buffer_size, 
                                (unsigned char*) buffer_decrypt);
                int read_size = cov_var->read(buffer_decrypt, client_y_size[client]);
                if (read_size != client_y_size[client]) {
                    throw ReadtsvERROR("covariant size mismatch from client: " +
                                       std::to_string(client) + " with cov: " + cov_var->name() + 
                                       " size expected: " + std::to_string(client_y_size[client]) + 
                                       " got: " + std::to_string(read_size));
                }
            }
            gwas->add_covariant(cov_var);
        }
    } catch (ERROR_t& err) {
        std::cerr << "ERROR: fail to get correct covariant values: " << err.msg << std::endl;
    } catch (const std::exception &e) { 
        std::cout << "Crash in cov setup with " << e.what() << std::endl;
    }
    std::cout << "Cov loaded" << std::endl;

    delete[] phenotype_buffer;
    delete[] buffer_decrypt;
    try {
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            buffer_list[thread_id]->add_gwas(gwas, impute_policy);
        }
    } catch (const std::exception &e) { 
        std::cout << "Crash in add gwas with " << e.what() << std::endl;
    }

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

    Buffer* buffer = buffer_list[thread_id];
    Batch* batch = nullptr;
    Row* row;

    /* process rows */
    while (true) {
        //start_timer("input()");
        if (!batch || batch->st != Batch::Working)
            batch = buffer->launch(client_info_list, thread_id);
        if (!batch) {
            buffer->clean_up();
            break;
        }
        //stop_timer("input()");
        // starting the get_row timer happens within the function because our code is written weirdly and we do our output
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
        //stop_timer("parse_and_decrypt()");
        //  compute results
        loci_to_str(row->getloci(), loci_string);
        alleles_to_str(row->getalleles(), alleles_string);
        output_string += loci_string + "\t" + alleles_string;
        //start_timer("kernel()");
        bool converge;
        //std::cout << i++ << std::endl;
        try {
            converge = row->fit();
            output_string += "\t" + std::to_string(row->get_beta()) +
                             "\t" + std::to_string(row->get_standard_error()) +
                             "\t" + std::to_string(row->get_t_stat());

            if (analysis_type == EncAnalysis::logistic) {
                output_string += + "\t" + std::to_string(row->get_iterations()) + "\t";
                // wanted to use a ternary, but the compiler doesn't like it?
                if (converge) {
                    output_string += "true";
                } else {
                    output_string += "false";
                }
            }
            output_string += "\n";
        } catch (MathError& err) {
            output_string += "\tNA\tNA\tNA\t1\tfalse\n";
            // cerr << "MathError while fiting " << ss.str() << ": " << err.msg
            //      << std::endl;
            // ss << "\tNA\tNA\tNA" << std::endl;
        } catch (ERROR_t& err) {
            std::cerr << "ERROR " << err.msg << std::endl << std::flush;
            // ss << "\tNA\tNA\tNA" << std::endl;
            exit(1);
        }
        //stop_timer("kernel()");
        batch->write(output_string);
        output_string.clear();
    }
}