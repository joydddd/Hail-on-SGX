#include <iostream>
#include <map>

#include "buffer.h"
#include "enc_gwas.h"
#include "logistic_regression.h"

using namespace std;

void log_regression(){
    cout << "Logistic Regression started" << endl;
    /* setup gwas */
    char clientl[ENCLAVE_READ_BUFFER_SIZE];
    getclientlist(clientl);
    vector<string> clients;
    string clientl_str(clientl);
    split_tab(clientl_str, clients);
    cout << "enclave running on " << clients.size() << " clients: " << endl;
    cout << clientl << endl;

    map<string, int> client_size_map;
    char* y_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    Log_var gwas_y;
    try {
        for (auto& client : clients) {
            bool rt = false;
            while (!rt) {
                gety(&rt, client.c_str(), y_buffer);
            }
            stringstream y_ss(y_buffer);
            Log_var new_y;
            new_y.read(y_ss);
            client_size_map[client] = new_y.size();
            gwas_y.combine(new_y);
        }
        delete[] y_buffer;
        cout << "Y value loaded" << endl;
    } catch (ERROR_t& err) {
        cerr << "ERROR: fail to get correct y values" << err.msg << endl;
    }

    Log_gwas gwas(gwas_y);

    char covl[ENCLAVE_READ_BUFFER_SIZE];
    getcovlist(covl);
    string covlist(covl);
    vector<string> covariants;
    split_tab(covlist, covariants);

    try {
        char* cov_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
        for (auto& cov : covariants) {
            if (cov == "1") {
                Log_var intercept(gwas_y.size());
                gwas.add_covariant(intercept);
                continue;
            }
            Log_var cov_var;
            for (auto& client : clients) {
                bool rt = false;
                while (!rt)
                    getcov(&rt, client.c_str(), cov.c_str(), cov_buffer);
                stringstream cov_ss(cov_buffer);
                Log_var new_cov_var;
                new_cov_var.read(cov_ss);
                if (new_cov_var.size() != client_size_map[client])
                    throw ReadtsvERROR("covariant size mismatch from client: " +
                                       client);
                cov_var.combine(new_cov_var);
            }
            gwas.add_covariant(cov_var);
        }
        delete[] cov_buffer;
        cout << "GWAS setup finished" << endl;
    } catch (ERROR_t& err) {
        cerr << "ERROR: fail to get correct covariant values: " << err.msg
             << endl;
    }

    Buffer buffer(client_size_map[clients[0]]);
    while (true) {
        // get the next row from input buffer
        Log_row* row;
        try {
            if (!(row = (Log_row*)buffer.launch())) break;
        } catch (ERROR_t& err) {
            cerr << "ERROR: " << err.msg << endl;
            continue;
        }

        // compute results
        ostringstream ss;
        ss << row->getloci() << "\t" << row->getallels();
        bool converge;
        try {
            converge = row->fit(&gwas);
            ss << "\t" << row->beta()[0] << "\t" << row->t_stat();
            ss << "\t" << (converge ? "true" : "false");
            ss << endl;
            // cout << ss.str();
        } catch (MathError& err) {
            cerr << "MathError while fiting " << ss.str() << ": " << err.msg
                 << endl;
            ss << "\tNA\tNA\tNA" << endl;
        } catch (ERROR_t& err) {
            cerr << "ERROR " << ss.str() << ": " << err.msg << endl;
            ss << "\tNA\tNA\tNA" << endl;
            exit(1);
        }
        cout << ss.str();
    }
    cout << "Logistic regression Finished! " << endl;
}