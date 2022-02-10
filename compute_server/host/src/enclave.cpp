#include <fstream>
#include <iostream>
#include <map>
#define BUFFER_LINES 1
#define OUTPUT_FILE "gwasoutput/writeback.out"

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

#include "gwas_u.h"
using namespace std;
const vector<vector<string>> covFiles = {
    {"../../samples/1kg-logistic-regression/isFemale1.tsv",
     "../../samples/1kg-logistic-regression/isFemale2.tsv"}};
const vector<string> yFiles = {
    "../../samples/1kg-logistic-regression/PurpleHair1.tsv",
    "../../samples/1kg-logistic-regression/PurpleHair2.tsv"};
const vector<string> allelesFiles = {
    "../../samples/1kg-logistic-regression/alleles1.tsv",
    "../../samples/1kg-logistic-regression/alleles2.tsv"};
const vector<string> clientNames = {"Client1", "Client2"};
const vector<int> client_size = {100, 150};

const vector<string> covNames = {"1", "isFemale"};

map<string, int> client_map;
map<string, int> cov_map;
// index -1 is reserved for intercept

static oe_enclave_t* enclave;

void setrsapubkey(uint8_t enc_rsa_pub_key[RSA_PUB_KEY_SIZE]) {
    std::memcpy(ComputeServer::get_rsa_pub_key(), enc_rsa_pub_key, RSA_PUB_KEY_SIZE);
    
    // Once we have generated an RSA key pair we can start communication!
    ComputeServer::finish_setup();
}

int getclientnum() {
    return ComputeServer::get_num_institutions();
}

void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]) {
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
    std::memcpy(y, &y_data[0], y_data.length());
    return y_data.length();
}

int getcov(const int client_num,
           const char cov_name[MAX_CLIENTNAME_LENGTH],
           char cov[ENCLAVE_READ_BUFFER_SIZE]) {
    if (strcmp(cov_name, "1") == 0) {
        strcpy(cov, "1");
        return 1;
    }
    std::string cov_data = ComputeServer::get_covariant_data(client_num, cov_name);
    if (!cov_data.length()) {
        return false;
    }
    std::memcpy(cov, &cov_data[0], cov_data.length());
    return cov_data.length();
}

int get_encrypted_x_size(const int client_num) {
    return ComputeServer::get_encypted_allele_size(client_num);
}

int getbatch(char batch[ENCLAVE_READ_BUFFER_SIZE], const int thread_id) {
    // TODO: maybe change this so we read in a diff number for each 
    std::string batch_data; 
    int num_lines = ComputeServer::get_allele_data(batch_data, thread_id);
    if (num_lines) {
        std::memcpy(batch, &batch_data[0], batch_data.length());
    }

    return num_lines;
}

void writebatch(Row_T type, char buffer[ENCLAVE_OUTPUT_BUFFER_SIZE]) {
    static ofstream result_f;
    if (!result_f.is_open()) {
        result_f.open(OUTPUT_FILE);
    }
    result_f << buffer;
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

int start_enclave() {
    oe_result_t result;
    int ret = 1;
    enclave = NULL;

    uint32_t flags = 0;

    if (ComputeServer::get_mode() == simulate) flags |= OE_ENCLAVE_FLAG_SIMULATE;
    if (ComputeServer::get_mode() == debug) flags |= OE_ENCLAVE_FLAG_DEBUG;

    // Create the enclave
    result = oe_create_gwas_enclave("../enclave/gwasenc.signed", OE_ENCLAVE_TYPE_AUTO, flags, NULL,
                                    0, &enclave);
    if (result != OE_OK) {
        fprintf(stderr, "oe_create_gwas_enclave(): result=%u (%s)\n", result,
                oe_result_str(result));
        goto exit;
    }

    try {
        std::cout << "\n\n**RUNNING LOG REGRESSION**\n\n";

        int num_threads = ComputeServer::get_num_threads();
        boost::thread_group thread_group;
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            boost::thread* enclave_thread = new boost::thread(log_regression, enclave, thread_id);
            thread_group.add_thread(enclave_thread);
        }

        result = setup_enclave(enclave, num_threads);
        if (result != OE_OK) {
            fprintf(stderr,
                    "calling into enclave_gwas failed: result=%u (%s)\n",
                    result, oe_result_str(result));
            goto exit;
        }

        auto start = std::chrono::high_resolution_clock::now();
        thread_group.join_all();
        // DEBUG: total execution time
        auto stop = std::chrono::high_resolution_clock::now();
        cout << "Logistic regression finished!" << endl;
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        cout << "Enclave time total: " << duration.count() << endl;
    } catch (ERROR_t& err) {
        cerr << "ERROR: " << err.msg << endl << std::flush;
    }

    ret = 0;

exit:
    // Clean up the enclave if we created one
    if (enclave) oe_terminate_enclave(enclave);

    return ret;
}