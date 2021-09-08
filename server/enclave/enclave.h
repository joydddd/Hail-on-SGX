#ifndef ENCLAVE_H
#define ENCLAVE_H

#include "../server_type.h"
#include "gwas.h"
#include "enc_gwas.h"

using namespace std;

class Gwas_Handler{
    public:
    vector<string> clients;
    map<string, int> client_size_map;
    bool clients_init = false;

    void setup_clientlist(char clientlist[ENCLAVE_READ_BUFFER_SIZE]);
};

class LogReg_Handler : public Gwas_Handler {
    bool cov_init = false;
    map<string, bool> y_inits;
    bool y_init = false;
    bool initialized = false;
    vector<string> covariants;
    GWAS_var gwas_y;
    GWAS_logic gwas;

   public:
    void init();
    void setup_covlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]);
    void setup_y(const string& clientname, char y[ENCLAVE_READ_BUFFER_SIZE]);

};

#endif