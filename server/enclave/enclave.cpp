#include "enclave.h"

#include <iostream>

#include "gwas_t.h"

static LogReg_Handler logreg_handler;

void log_regression() { logreg_handler.init(); }

void linear_regression_beta() {}

void linear_regression_t_stat() {}

void LogReg_Handler::init(){
    if (initialized) return;
    while (clients_init == false) req_clientlist();
    while(cov_init == false) req_covlist();
    while(y_init == false){
        if(y_inits.empty()){
            for (auto& client:clients){
                req_y(client.c_str());
            }
        } else {
            for(auto& client_y:y_inits){
                if (client_y.second == false) req_y(client_y.first.c_str());
            }
        }
    }
    cout << "Enclave intialized for log regression." << endl;
    initialized = true;
}

bool input_clientlist(const char clientlist[ENCLAVE_READ_BUFFER_SIZE]) {
    char enc_clientlist[ENCLAVE_READ_BUFFER_SIZE];
    strncpy(enc_clientlist, clientlist, ENCLAVE_READ_BUFFER_SIZE);
    cout << "Client List: \nclientname\tsize\n"
         << clientlist << endl;
    try {
        logreg_handler.setup_clientlist(enc_clientlist);
    } catch (ERROR_t& err) {
        cerr << "ERROR: " << err.msg << endl;
        return false;
    }
    return true;
}

void Gwas_Handler::setup_clientlist(char clientlist[ENCLAVE_READ_BUFFER_SIZE]) {
    if (clients_init) throw EncInitERROR("multiple initlization of clientlist");
    stringstream clientname_ss(clientlist);
    string line;
    while (getline(clientname_ss, line)) {
        vector<string> parts;
        split_tab(line, parts);
        if (parts.size() != 2)
            throw ReadtsvERROR("invalid client list: " + line);
        int size;
        try {
            size = stoi(parts[1]);
        } catch (invalid_argument& err) {
            throw ReadtsvERROR("invalid client size: " + line);
        }
        clients.push_back(parts[0]);
        client_size_map.insert(make_pair(parts[0], size));
    }
    clients_init = true;
    cout << "Working on " << clients.size() << " cliens: \nclientname\tsize\n"
         << clientlist << endl;
}

bool input_covlist(const char covlist[ENCLAVE_READ_BUFFER_SIZE]) {
    char enc_covlist[ENCLAVE_READ_BUFFER_SIZE];
    strncpy(enc_covlist, covlist, ENCLAVE_READ_BUFFER_SIZE);
    try {
        logreg_handler.setup_covlist(enc_covlist);
    } catch (ERROR_t& err) {
        cerr << "ERROR: " << err.msg << endl;
        return false;
    }
    return true;
}

void LogReg_Handler::setup_covlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]) {
    if (cov_init) throw EncInitERROR("multiple initlization of covariant list");
    string covlist_str(covlist);
    split_tab(covlist_str, covariants);
    cov_init = true;
    cout << "Taking " << covariants.size() << " covariants: " << covlist
         << endl;
}

bool input_y(const char clientname[MAX_CLIENTNAME_LENGTH],
             const char y[ENCLAVE_READ_BUFFER_SIZE]) {
    char enc_y[ENCLAVE_READ_BUFFER_SIZE];
    char enc_clientname[MAX_CLIENTNAME_LENGTH];
    strncpy(enc_y, y, ENCLAVE_READ_BUFFER_SIZE);
    strncpy(enc_clientname, clientname, MAX_CLIENTNAME_LENGTH);
    string clientname_str(enc_clientname);
    try {
        logreg_handler.setup_y(clientname_str, enc_y);
    } catch (ERROR_t& err) {
        cerr << "ERROR: " << err.msg << endl;
        return false;
    }
    return true;
}

void LogReg_Handler::setup_y(const string& clientname,
                             char y[ENCLAVE_READ_BUFFER_SIZE]) {
    if (!clients_init)
        throw EncInitERROR("clientlist must be intialized before y!");
    if (y_init) throw EncInitERROR("multiple initialization of y ");
    if (y_inits.empty()) {
        for (auto& client : clients) {
            y_inits.insert(make_pair(client, false));
        }
    }
    auto found = y_inits.find(clientname);
    if (found == y_inits.end())
        throw EncInitERROR("unknow client name " + clientname);
    if (found->second)
        throw EncInitERROR("multiple initialization of y from client " +
                           clientname);

    stringstream y_ss(y);
    GWAS_var new_y;
    new_y.read(y_ss);
    if (new_y.size() != client_size_map[clientname])
        throw ReadtsvERROR("y size mismatch from client: " + clientname);
    gwas_y.combine(new_y);
    found->second = true;

    // checks if y initialization is finished
    for (auto& client_y_init : y_inits) {
        if (client_y_init.second == false) return;
    }
    gwas.add_y(gwas_y);
    y_init = true;
    cout << "y initialization finished" << endl;
}
