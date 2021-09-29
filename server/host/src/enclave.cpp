#include <fstream>
#include <iostream>
#include <map>
#define BUFFER_LINES 10
#define OUTPUT_FILE "test_logistic.out"

#include <gwas.h>

#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

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
void init() {
    for (int i = 0; i < clientNames.size(); i++) {
        client_map.insert(make_pair(clientNames[i], i));
    }
    int j = 0;
    for (int i = 0; i < covNames.size(); i++) {
        if (covNames[i] == "1") {
            cov_map.insert(make_pair("1", -1));
        } else {
            cov_map.insert(make_pair(covNames[i], j));
            j++;
        }
    }
}

void getclientlist(char clientlist[ENCLAVE_READ_BUFFER_SIZE]) {
    stringstream list_ss;
    for (size_t i = 0; i < clientNames.size(); i++) {
        list_ss << clientNames[i] << "\t" << client_size[i] << "\n";
    }
    strcpy(clientlist, list_ss.str().c_str());
}

void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]) {
    stringstream ss;
    for (auto& cov : covNames) {
        ss << cov << "\t";
    }
    strcpy(covlist, ss.str().c_str());
}

void gety(const char client[MAX_CLIENTNAME_LENGTH],
          char y[ENCLAVE_READ_BUFFER_SIZE]) {
    static vector<ifstream> y_fstreams;
    if (y_fstreams.empty()) {
        for (auto& y_file : yFiles) {
            y_fstreams.push_back(ifstream(y_file));
            if (!y_fstreams.back().is_open())
                throw ReadtsvERROR("fail to open file " + y_file);
        }
    }
    string client_name(client);
    int index = client_map[client_name];
    stringstream ss;
    ss << y_fstreams[index].rdbuf();
    strcpy(y, ss.str().c_str());
}

void getcov(const char client[MAX_CLIENTNAME_LENGTH],
            const char cov_name[MAX_CLIENTNAME_LENGTH],
            char cov[ENCLAVE_READ_BUFFER_SIZE]) {
    if (cov_name == "1") {
        strcpy(cov, "1");
        return;
    }
    static vector<vector<ifstream>> cov_streams;
    if (cov_streams.empty()) {
        for (auto& cov : covNames) {
            if (cov == "1") continue;
            cov_streams.push_back(vector<ifstream>());
            for (auto& cov_file : covFiles[cov_map[cov]]) {
                cov_streams[cov_map[cov]].push_back(ifstream(cov_file));
                if (!cov_streams[cov_map[cov]].back().is_open())
                    throw ReadtsvERROR("fail to open file " + cov_file);
            }
        }
    }
    string cov_str(cov_name);
    string client_str(client);
    int cov_index = cov_map[cov_str];
    int client_index = client_map[client_str];
    stringstream ss;
    ss << cov_streams[cov_index][client_index].rdbuf();
    strcpy(cov, ss.str().c_str());
}

bool getbatch(const char client[MAX_CLIENTNAME_LENGTH], Row_T type,
              char batch[ENCLAVE_READ_BUFFER_SIZE]) {
    static vector<fstream> alleles_stream;
    if (alleles_stream.empty()) {
        for (auto& fname : allelesFiles) {
            alleles_stream.push_back(fstream(fname));
            if (!alleles_stream.back().is_open()) {
                throw ReadtsvERROR("fail to open file " + fname);
            }
            string first_line;
            getline(alleles_stream.back(), first_line);
            // throw away the header line
        }
    }
    string client_str(client);
    int index = client_map[client_str];
    if (alleles_stream[index].eof()) {
        strcpy(batch, EndSperator);
        return true;
    }
    stringstream buffer_ss;
    for (size_t i = 0; i < BUFFER_LINES; i++) {
        string line;
        if (!getline(alleles_stream[index], line)) break;
        buffer_ss << line << "\n";
    }
    if (buffer_ss.str() == "\n") {
        strcpy(batch, EndSperator);
        return true;
    }
    strcpy(batch, buffer_ss.str().c_str());
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

int setupenclave(int argc, const char* argv[]) {
    oe_result_t result;
    int ret = 1;
    enclave = NULL;

    uint32_t flags = OE_ENCLAVE_FLAG_DEBUG;
    if (check_simulate_opt(&argc, argv)) {
        flags |= OE_ENCLAVE_FLAG_SIMULATE;
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: %s enclave_image_path [ --simulate  ]\n",
                argv[0]);
        goto exit;
    }

    // Create the enclave
    result = oe_create_gwas_enclave(argv[1], OE_ENCLAVE_TYPE_AUTO, flags, NULL,
                                    0, &enclave);
    if (result != OE_OK) {
        fprintf(stderr, "oe_create_gwas_enclave(): result=%u (%s)\n", result,
                oe_result_str(result));
        goto exit;
    }

    try {
        init();
        result = log_regression(enclave);
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