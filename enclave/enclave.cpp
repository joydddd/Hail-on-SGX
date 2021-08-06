#include "enclave.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "gwas.h"

// TODO: impliment decryption in enclave
void enclave_decrypt(char crypt[ENCLAVE_READ_BUFFER_SIZE], string& plaintxt) {
    plaintxt = crypt;
}

void log_regression(bool intercept) {
    cout << "Logistic Regression started" << endl;
    /* setup gwas */
    char hostl[ENCLAVE_READ_BUFFER_SIZE];
    gethostlist(hostl);
    vector<string> hosts;
    vector<int> host_sizes;
    stringstream hostname_ss(hostl);
    string line;
    while (getline(hostname_ss, line)) {
        vector<string> parts;
        split_tab(line, parts);
        if (parts.size() != 2) throw ReadtsvERROR("invalid host list: " + line);
        int size;
        try {
            size = stoi(parts[1]);
        } catch (invalid_argument& err) {
            throw ReadtsvERROR("invalid host size: " + line);
        }
        hosts.push_back(parts[0]);
        host_sizes.push_back(size);
    }
    char* y_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    GWAS_var gwas_y;
    for (auto& host : hosts) {
        gety(host.c_str(), y_buffer);
        stringstream y_ss(y_buffer);
        GWAS_var new_y;
        new_y.read(y_ss);
        gwas_y.combine(new_y);
    }
    delete y_buffer;

    GWAS_logic gwas(gwas_y);

    char covl[ENCLAVE_READ_BUFFER_SIZE];
    getcovlist(covl);
    string covlist(covl);
    vector<string> covariants;
    split_tab(covlist, covariants);

    char* cov_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    for (auto& cov : covariants) {
        if (cov == "1") {
            GWAS_var intercept(gwas_y.size());
            gwas.add_covariant(intercept);
            continue;
        }
        GWAS_var cov_var;
        for (auto& host : hosts) {
            getcov(host.c_str(), cov.c_str(), cov_buffer);
            stringstream cov_ss(cov_buffer);
            GWAS_var new_cov_var;
            new_cov_var.read(cov_ss);
            cov_var.combine(new_cov_var);
        }
        gwas.add_covariant(cov_var);
    }
    cout << "GWAS setup finished" << endl;

    /* setup read buffer */
    OutputBuffer result_buffer(Result_t);
    Buffer raw_data(LOG_t);
    for (size_t i = 0; i < hosts.size(); i++) {
        raw_data.add_host(hosts[i], host_sizes[i]);
    }
    raw_data.init();

    /* setupt output buffer */
    result_buffer.extend("locus\talleles\tbeta\tt_stat\tcoverged\n");

    cout << "Buffer initialized" << endl;

    /* process rows */
    GWAS_row* row;
    while (true) {
        try {
            if (!(row = (GWAS_row*)raw_data.get_nextrow(gwas))) break;
        } catch (ERROR_t& err) {
            cerr << "ERROR: " << err.msg << endl;
            continue;
        }
        row->init();
        ostringstream ss;
        ss << row->getloci() << "\t" << row->getallels();
        bool converge;
        try {
            converge = row->fit();
            ss << "\t" << row->beta()[0] << "\t" << row->t_stat();
            ss << "\t" << (converge ? "true" : "false");
            ss << endl;
        } catch (MathError& err) {
            cerr << "MathError while fiting " << ss.str() << ": " << err.msg
                 << endl;
            ss << "\tNA\tNA\tNA" << endl;
            delete row;
            continue;
        } catch (ERROR_t& err) {
            cerr << "ERROR " << ss.str() << ": " << err.msg << endl;
            ss << "\tNA\tNA\tNA" << endl;
            delete row;
            exit(1);
            continue;
        }
        // cerr << ss.str();
        while (!result_buffer.extend(ss.str())) {
            // result_buffer.print();
            writebatch(Result_t, result_buffer.copy_to_host());
        }
        delete row;
    }
    // result_buffer.print();
    writebatch(Result_t, result_buffer.copy_to_host());
    cout << "Logistic regression Finished! " << endl;
}
