#include <fstream>
#include <iostream>
#include <map>
#define BUFFER_LINES 1
#define OUTPUT_FILE "writeback.out"

#include <gwas.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include "server.h"

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


void getclientlist(char clientlist[ENCLAVE_READ_BUFFER_SIZE]) {
    strcpy(clientlist, Server::get_institutions().c_str());
}

void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]) {
    strcpy(covlist, Server::get_covariants().c_str());
}

bool gety(const char client[MAX_CLIENTNAME_LENGTH],
          char y[ENCLAVE_READ_BUFFER_SIZE]) {
    std::string y_data = Server::get_y_data(client);
    if (y_data == "") {
        return false;
    }
    strcpy(y, y_data.c_str());
    return true;
}

bool getcov(const char client[MAX_CLIENTNAME_LENGTH],
            const char cov_name[MAX_CLIENTNAME_LENGTH],
            char cov[ENCLAVE_READ_BUFFER_SIZE]) {
    if (cov_name == "1") {
        strcpy(cov, "1");
        return true;
    }
    std::string cov_data = Server::get_covariant_data(client, cov_name);
    if (cov_data == "") {
        return false;
    }
    strcpy(cov, cov_data.c_str());
    return true;
}

bool getbatch(const char client[MAX_CLIENTNAME_LENGTH], Row_T type,
              char batch[ENCLAVE_READ_BUFFER_SIZE]) {
    // TODO: maybe change this so we read in a diff number for each 
    std::string batch_data = Server::get_x_data(client, BUFFER_LINES);
    if (batch_data == "") {
        return false;
    }
    strcpy(batch, batch_data.c_str());
    return true;
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

int start_enclave(int argc, const char* argv[]) {
    oe_result_t result;
    int ret = 1;
    enclave = NULL;

    uint32_t flags = 0;
    if (check_simulate_opt(&argc, argv)) {
        flags |= OE_ENCLAVE_FLAG_SIMULATE;
    }

    if (argc != 3) {
        fprintf(stderr, "Usage: %s enclave_image_path [ --simulate  ]\n",
                argv[0]);
        goto exit;
    }

    // Create the enclave
    result = oe_create_gwas_enclave(argv[2], OE_ENCLAVE_TYPE_AUTO, flags, NULL,
                                    0, &enclave);
    if (result != OE_OK) {
        fprintf(stderr, "oe_create_gwas_enclave(): result=%u (%s)\n", result,
                oe_result_str(result));
        goto exit;
    }

    try {
        std::cout << "\n\n**RUNNING LOG REGRESSION**\n\n";
        // DEBUG:
        auto start = std::chrono::high_resolution_clock::now();
        result = log_regression(enclave);
        // DEBUG: total execution time
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        cout << "Enclave time total: " << duration.count() << endl;
        if (result != OE_OK) {
            fprintf(stderr,
                    "calling into enclave_gwas failed: result=%u (%s)\n",
                    result, oe_result_str(result));
            goto exit;
        }
    } catch (ERROR_t& err) {
        cerr << "ERROR: " << err.msg << endl;
    }

    ret = 0;

exit:
    // Clean up the enclave if we created one
    if (enclave) oe_terminate_enclave(enclave);

    return ret;
}