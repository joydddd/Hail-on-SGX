#include <fstream>
#include <iostream>
#include <map>
#define BUFFER_LINES 1

#include <gwas.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <stdio.h>
#include <boost/thread.hpp>
#include "compute_server.h"

#define MAX_ATTEMPT_TIMES 10
#define ATTEMPT_TIMEOUT 500  // in milliseconds

#ifdef NON_OE
#include "host_glue.h"
#include "../../enclave/include/enclave_glue.h"
#else
#include "gwas_u.h"
#endif


;
// index -1 is reserved for intercept


void setrsapubkey(uint8_t enc_rsa_pub_key[RSA_PUB_KEY_SIZE]) {
    std::memcpy(ComputeServer::get_rsa_pub_key(), enc_rsa_pub_key, RSA_PUB_KEY_SIZE);
    
    // Once we have generated an RSA key pair we can start communication!
    ComputeServer::finish_setup();
}

void setmaxbatchlines(int lines) {
    ComputeServer::set_max_batch_lines(lines);
}

void start_timer(const char func_name[MAX_CLIENTNAME_LENGTH]) {
    ComputeServer::start_timer(func_name);
}

void stop_timer(const char func_name[MAX_CLIENTNAME_LENGTH]) {
    ComputeServer::stop_timer(func_name);
}

int getclientnum() {
    return ComputeServer::get_num_institutions();
}

void getcovlist(char covlist[ENCLAVE_SMALL_BUFFER_SIZE]) {
    std::memset(covlist, 0, ENCLAVE_SMALL_BUFFER_SIZE);
    strcpy(covlist, ComputeServer::get_covariants().c_str());
}

bool getaes(const int client_num,
            const int thread_id,
            unsigned char key[256],
            unsigned char iv[256]) {
    std::string encrypted_aes_key = ComputeServer::get_aes_key(client_num, thread_id);
    std::string encrypted_aes_iv = ComputeServer::get_aes_iv(client_num, thread_id);
    if (!encrypted_aes_key.length() || !encrypted_aes_iv.length()) {
        return false;
    }
    std::memcpy(key, &encrypted_aes_key[0], 256);
    std::memcpy(iv, &encrypted_aes_iv[0], 256);
    return true;
}

int gety(const int client_num, char y[ENCLAVE_READ_BUFFER_SIZE]) {
    std::string y_data = ComputeServer::get_y_data(client_num);
    if (!y_data.length()) {
        return 0;
    }
    std::memset(y, 0, ENCLAVE_READ_BUFFER_SIZE);
    std::memcpy(y, &y_data[0], y_data.length());
    return y_data.length();
}

int getcov(const int client_num,
           const char cov_name[MAX_CLIENTNAME_LENGTH],
           char cov[ENCLAVE_READ_BUFFER_SIZE]) {
    if (strcmp(cov_name, "1") == 0) {
        std::memset(cov, 0, ENCLAVE_READ_BUFFER_SIZE);
        strcpy(cov, "1");
        return 1;
    }
    std::string cov_data = ComputeServer::get_covariant_data(client_num, cov_name);
    if (!cov_data.length()) {
        return 0;
    }
    std::memset(cov, 0, ENCLAVE_READ_BUFFER_SIZE);
    if (cov_data.length() > ENCLAVE_READ_BUFFER_SIZE) {
        std::cout << "cov data length " << ENCLAVE_READ_BUFFER_SIZE << std::endl;
    }
    std::memcpy(cov, &cov_data[0], cov_data.length());

    return cov_data.length();
}

int get_encrypted_x_size(const int client_num) {
    return ComputeServer::get_encrypted_allele_size(client_num);
}

int getbatch(char batch[ENCLAVE_READ_BUFFER_SIZE], const int thread_id) {
    return ComputeServer::get_allele_data(batch, thread_id);
}

void writebatch(Row_T type, char buffer[ENCLAVE_READ_BUFFER_SIZE], const int buffer_size, const int thread_id) {
    ComputeServer::write_allele_data(buffer, buffer_size, thread_id);
}

bool check_simulate_opt(int* argc, const char* argv[]) {
    for (int i = 0; i < *argc; i++) {
        if (strcmp(argv[i], "--simulate") == 0) {
            fprintf(stdout, "Running in simulation mode\n");
            memmove(&argv[i], &argv[i + 1], (*argc - i) * sizeof(char*));
            (*argc)--;
            return true;
        }
    }
    return false;
}

bool check_debug_opt(int* argc, const char* argv[]) {
    for (int i = 0; i < *argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            fprintf(stdout, "Running in debug mode\n");
            memmove(&argv[i], &argv[i + 1], (*argc - i) * sizeof(char*));
            (*argc)--;
            return true;
        }
    }
    return false;
}

#ifndef NON_OE

static oe_enclave_t* enclave;

int start_enclave() {
    oe_result_t result;
    int ret = 1;
    enclave = NULL;

    uint32_t flags = 0;

    if (ComputeServer::get_mode() == simulate) flags |= OE_ENCLAVE_FLAG_SIMULATE;
    if (ComputeServer::get_mode() == debug) flags |= OE_ENCLAVE_FLAG_DEBUG;

    EncAnalysis enc_analysis_type = ComputeServer::get_analysis();

    // Create the enclave
    result = oe_create_gwas_enclave("../enclave/gwasenc.signed", OE_ENCLAVE_TYPE_AUTO, flags, NULL,
                                    0, &enclave);
    if (result != OE_OK) {
        fprintf(stderr, "oe_create_gwas_enclave(): result=%u (%s)\n", result,
                oe_result_str(result));
        goto exit;
    }

    try {
        std::cout << "\n\n**RUNNING REGRESSION**\n\n";

        int num_threads = ComputeServer::get_num_threads();
        boost::thread_group thread_group;
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            boost::thread* enclave_thread = new boost::thread(regression, enclave, thread_id, enc_analysis_type);
            thread_group.add_thread(enclave_thread);
        }

        result = setup_enclave_encryption(enclave, num_threads);
        if (result != OE_OK) {
            fprintf(stderr,
                    "calling into enclave_gwas failed: result=%u (%s)\n",
                    result, oe_result_str(result));
            goto exit;
        }
        
        // Allow for data to be encrypted beforehand
        char* tmp = new char[ENCLAVE_READ_BUFFER_SIZE];
        for (int client_id = 0; client_id < getclientnum(); ++client_id) {
            while(!gety(client_id, tmp)) {}
        }
        delete[] tmp;
        std::cout << "Starting Enclave: "  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "\n";
        auto start = std::chrono::high_resolution_clock::now();

        result = setup_enclave_phenotypes(enclave, num_threads, enc_analysis_type);
        if (result != OE_OK) {
            fprintf(stderr,
                    "calling into enclave_gwas failed: result=%u (%s)\n",
                    result, oe_result_str(result));
            goto exit;
        }

        
        thread_group.join_all();

        auto stop = std::chrono::high_resolution_clock::now();
        std::cout << "Logistic regression finished!" << std::endl;
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Enclave time total: " << duration.count() << std::endl;
        ComputeServer::print_timings();
        ComputeServer::cleanup_output();
    } catch (ERROR_t& err) {
        std::cerr << "ERROR: " << err.msg << std::endl << std::flush;
    }

    ret = 0;

exit:
    // Clean up the enclave if we created one
    if (enclave) oe_terminate_enclave(enclave);

    return ret;
}

#else


int start_enclave() {
    int ret = 1;

    try {
        std::cout << "\n\n**RUNNING LOG REGRESSION**\n\n";

        EncAnalysis enc_analysis_type = ComputeServer::get_analysis();
        int num_threads = 1;
        boost::thread_group thread_group;

        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            boost::thread* enclave_thread = new boost::thread(regression, thread_id, enc_analysis_type);
            thread_group.add_thread(enclave_thread);
        }

        setup_enclave_encryption(num_threads);

        char* tmp = new char[ENCLAVE_READ_BUFFER_SIZE];
        for (int client_id = 0; client_id < getclientnum(); ++client_id) {
            while(!gety(client_id, tmp)) {}
        }
        delete[] tmp;

        auto start = std::chrono::high_resolution_clock::now();

        setup_enclave_phenotypes(num_threads, enc_analysis_type);
        thread_group.join_all();

        auto stop = std::chrono::high_resolution_clock::now();
        std::cout << "Logistic regression finished!" << std::endl;
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Enclave time total: " << duration.count() << std::endl;
        ComputeServer::print_timings();
        ComputeServer::cleanup_output();
    } catch (ERROR_t& err) {
        std::cerr << "ERROR: " << err.msg << std::endl << std::flush;
    }

    ret = 0;

exit:
    return ret;
}


#endif