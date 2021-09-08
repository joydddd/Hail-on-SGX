#include <fstream>
#include <iostream>
#include <map>
#define BUFFER_LINES 10
#define OUTPUT_FILE "test_logistic.out"

#include <cstring>
#include <vector>
#include <string>
#include <gwas.h>
#include <chrono>
#include <thread>

#define MAX_ATTEMPT_TIMES 10
#define ATTEMPT_TIMEOUT 500 // in milliseconds

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

void req_clientlist() {
    cerr << "Request to send clientlist" << endl;
    stringstream list_ss;
    for (size_t i = 0; i < clientNames.size(); i++) {
        list_ss << clientNames[i] << "\t" << client_size[i] << "\n";
    }
    int i = 0;
    oe_result_t result;
    while (true) {
        bool ret;
        result = input_clientlist(enclave, &ret, list_ss.str().c_str());
        cerr << "sent clientlist" << endl;
        if (result != OE_OK) throw EncERROR("input_clientlist failed");
        if (ret) return;
        this_thread::sleep_for(chrono::milliseconds(ATTEMPT_TIMEOUT));
        i++;
        if (i >= 10) {
            cerr << "Host: fail to input clientlist after " << MAX_ATTEMPT_TIMES
                 << " attempts" << endl;
        }
    }
}

void req_covlist(){
    stringstream ss;
    for (auto& cov : covNames) {
        ss << cov << "\t";
    }
    oe_result_t result;
    int i = 0;
    while (true) {
        bool ret;
        result = input_covlist(enclave, &ret, ss.str().c_str());
        if (result != OE_OK) throw EncERROR("input_covlist failed");
        if (ret) return;
        this_thread::sleep_for(chrono::milliseconds(ATTEMPT_TIMEOUT));
        i++;
        if (i >= 10) {
            cerr << "Host: fail to input covariants list after " << MAX_ATTEMPT_TIMES
                 << " attempts" << endl;
        }
    }
}

void req_y(const char client[MAX_CLIENTNAME_LENGTH]) {
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


    oe_result_t result;
    int i = 0;
    while (true) {
        bool ret;
        result = input_y(enclave, &ret, client_name.c_str(), ss.str().c_str());
        if (result != OE_OK) throw EncERROR("input_y failed");
        if (ret) return;
        this_thread::sleep_for(chrono::milliseconds(ATTEMPT_TIMEOUT));
        i++;
        if (i >= 10) {
            cerr << "Host: fail to input y after "
                 << MAX_ATTEMPT_TIMES << " attempts" << endl;
        }
    }
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

int main(int argc, const char* argv[]) {
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
    result = oe_create_gwas_enclave(argv[1], OE_ENCLAVE_TYPE_AUTO, flags,
                                      NULL, 0, &enclave);
    if (result != OE_OK) {
        fprintf(stderr, "oe_create_gwas_enclave(): result=%u (%s)\n",
                result, oe_result_str(result));
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
    } catch (SysERROR& err) {
        cerr << "ERROR: " << err.msg << endl;
        goto exit;
    }

    ret = 0;

exit:
    // Clean up the enclave if we created one
    if (enclave) oe_terminate_enclave(enclave);

    return ret;

}