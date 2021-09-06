#include <fstream>
#include <iostream>
#include <map>
#define BUFFER_LINES 10
#define OUTPUT_FILE "test_logistic.out"

#include "enclave.h"
#include <vector>
#include <cstring>

const vector<vector<string>> covFiles = {
    {"../../samples/1kg-logistic-regression/isFemale1.tsv",
     "../../samples/1kg-logistic-regression/isFemale2.tsv"}};
const vector<string> yFiles = {
    "../../samples/1kg-logistic-regression/PurpleHair1.tsv",
    "../../samples/1kg-logistic-regression/PurpleHair2.tsv"};
const vector<string> allelesFiles = {
    "../../samples/1kg-logistic-regression/alleles1.tsv",
    "../../samples/1kg-logistic-regression/alleles2.tsv"};
const vector<string> hostNames = {"Host1", "Host2"};
const vector<int> host_size = {100, 150};

const vector<string> covNames = {"1", "isFemale"};

using namespace std;

map<string, int> host_map;
map<string, int> cov_map; 
// index -1 is reserved for intercept
void init() {
    for (int i = 0; i < hostNames.size(); i++) {
        host_map.insert(make_pair(hostNames[i], i));
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

void getbatch(const char hostname[MAX_HOST_LENGTH], Row_T type,
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
    string host(hostname);
    int index = host_map[host];
    if (alleles_stream[index].eof()) {
        strcpy(batch, EndSperator);
        return;
    }
    stringstream buffer_ss;
    for (size_t i = 0; i < BUFFER_LINES; i++) {
        string line;
        if (!getline(alleles_stream[index], line)) break;
        buffer_ss << line << "\n";
    }
    if (buffer_ss.str() == "\n") {
        strcpy(batch, EndSperator);
        return;
    }
    strcpy(batch, buffer_ss.str().c_str());
}

void writebatch(Row_T type, char buffer[ENCLAVE_OUTPUT_BUFFER_SIZE]) {
    static ofstream result_f;
    if (!result_f.is_open()){
        result_f.open(OUTPUT_FILE);
    }
    result_f << buffer;
}

void gethostlist(char hostlist[ENCLAVE_READ_BUFFER_SIZE]) {
    stringstream list_ss;
    for (size_t i = 0; i < hostNames.size(); i++) {
        list_ss << hostNames[i] << "\t" << host_size[i] << "\n";
    }
    strcpy(hostlist, list_ss.str().c_str());
}

void gety(const char host[MAX_HOST_LENGTH], char y[ENCLAVE_READ_BUFFER_SIZE]) {
    static vector<ifstream> y_fstreams;
    if (y_fstreams.empty()) {
        for (auto& y_file : yFiles) {
            y_fstreams.push_back(ifstream(y_file));
            if (!y_fstreams.back().is_open())
                throw ReadtsvERROR("fail to open file " + y_file);
        }
    }
    string host_name(host);
    int index = host_map[host_name];
    stringstream ss;
    ss << y_fstreams[index].rdbuf();
    strcpy(y, ss.str().c_str());
}

void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]) {
    stringstream ss;
    for(auto& cov:covNames){
        ss << cov << "\t";
    }
    strcpy(covlist, ss.str().c_str());
}

void getcov(const char host[MAX_HOST_LENGTH], const char cov_name[MAX_HOST_LENGTH],
            char cov[ENCLAVE_READ_BUFFER_SIZE]) {
    if (cov_name == "1") {
        strcpy(cov, "1");
        return;
    }
    static vector<vector<ifstream>> cov_streams;
    if (cov_streams.empty()) {
        for(auto& cov:covNames){
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
    string host_str(host);
    int cov_index = cov_map[cov_str];
    int host_index = host_map[host_str];
    stringstream ss;
    ss << cov_streams[cov_index][host_index].rdbuf();
    strcpy(cov, ss.str().c_str());
}

int main() {
    try {
        init();
        log_regression();
    } catch (ERROR_t& err) {
        cerr << err.msg << endl;
    }
}