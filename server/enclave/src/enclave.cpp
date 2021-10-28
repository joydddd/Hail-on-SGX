#include <chrono>
#include <iostream>
#include <thread>

#include "buffer.h"

#ifdef ENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif


void log_regression() {
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
        while (!rt){
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

    try{
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
    }
    catch (ERROR_t& err) {
        cerr << "ERROR: fail to get correct covariant values: " << err.msg << endl;
    }

    /* setup read buffer */
    Buffer raw_data(LOG_t);
    for (auto& client : clients) {
        raw_data.add_client(client, client_size_map[client]);
    }
    raw_data.init();

    /* setupt output buffer */
    OutputBuffer result_buffer(Result_t);
    result_buffer.write("locus\talleles\tbeta\tt_stat\tcoverged\n");

    cout << "Buffer initialized" << endl;

    /* process rows */
    Log_row* row;
    while (true) {
        // get the next row from input buffer
        try {
            if (!(row = (Log_row*)raw_data.get_nextrow(gwas))) break;
        } catch (ERROR_t& err) {
            cerr << "ERROR: " << err.msg << endl;
            continue;
        }

        // compute results
        ostringstream ss;
        ss << row->getloci() << "\t" << row->getallels();
        bool converge;
        try {
            converge = row->fit();
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
            delete row;
            exit(1);
        }
        result_buffer.write(ss.str());
        delete row;
    }
    result_buffer.writeback();

    cout << "Logistic regression Finished! " << endl;
}

void linear_regression_beta() {
    cout << "Linear regression beta started" << endl;
    // /* get client and covariant list*/
    // char clientl[ENCLAVE_READ_BUFFER_SIZE];
    // getclientlist(clientl);
    // vector<string> clients;
    // stringstream clientname_ss(clientl);
    // string line;
    // while (getline(clientname_ss, line)) {
    //     clients.push_back(line);
    // }

    // char covl[ENCLAVE_READ_BUFFER_SIZE];
    // getcovlist(covl);
    // string covlist(covl);
    // vector<string> covariants;
    // split_tab(covlist, covariants);
    // size_t dim = covariants.size() + 1;

    // /*setup read buffer */
    // Buffer xtx_buffer(XTX_t), xty_buffer(XTY_t);
    // for(auto& client:clients){
    //     xtx_buffer.add_client(client);
    //     xty_buffer.add_client(client);
    // }
    // xtx_buffer.init();
    // xty_buffer.init();

    // /* setup Output buffer */
    // OutputBuffer beta_buffer(BETA_t);
    // stringstream titles_ss;
    // titles_ss << "locus\talleles";
    // for (size_t i = 0; i < dim; i++) {
    //     titles_ss << "\tbeta" << i;
    // }
    // titles_ss << "\n";
    // beta_buffer.extend(titles_ss.str());

    // cout << "Beta Bufer initialized" << endl;

    // /* process rows */
    // XTX_row* xtx_row;
    // XTY_row* xty_row;
    // while(true){
    //     try{
    //         if (!(xtx_row = (XTX_row*)xtx_buffer.get_nextrow())) break;
    //         if (!(xty_row = (XTY_row*)xty_buffer.get_nextrow())) break;
    //     } catch(ERROR_t &err){
    //         cerr << "ERROR: " << err.msg << endl;
    //         continue;
    //     }
    //     ostringstream ss;
    //     ss << xtx_row->getloci() << "\t" << xtx_row->getalleles();
    //     try {
    //         vector<double> beta;
    //         xtx_row->beta(beta, *xty_row);
    //         for(auto b:beta){
    //             ss << "\t" << b;
    //         }
    //     } catch (ERROR_t& err) {
    //         cerr << "ERROR while processing " << ss.str() << ": " << err.msg
    //              << endl;
    //         for (size_t i = 0; i<dim; i++){
    //             ss << "\tNA";
    //         }
    //     }
    //     while(!beta_buffer.extend(ss.str())){
    //         writebatch(BETA_t, beta_buffer.copy_to_host());
    //     }
    //     delete xtx_row;
    //     delete xty_row;
    // }
    // writebatch(BETA_t, beta_buffer.copy_to_host());
    // cout << "Linear regression beta calculation finished!" << endl;
}

void linear_regression_t_stat() {
    cout << "Linear regression t_stat started" << endl;
//     /* get client and covariant list*/
//     char clientl[ENCLAVE_READ_BUFFER_SIZE];
//     getclientlist(clientl);
//     vector<string> clients;
//     stringstream clientname_ss(clientl);
//     string line;
//     while (getline(clientname_ss, line)) {
//         clients.push_back(line);
//     }

//     char covl[ENCLAVE_READ_BUFFER_SIZE];
//     getcovlist(covl);
//     string covlist(covl);
//     vector<string> covariants;
//     split_tab(covlist, covariants);
//     size_t dim = covariants.size() + 1;

//     /*setup read buffer */
//     Buffer sse_buffer(SSE_t), xtx_buffer(XTX_t);
//     for (auto& client : clients) {
//         sse_buffer.add_client(client);
//         xtx_buffer.add_client(client);
//     }
//     sse_buffer.init();
//     xtx_buffer.init();

//     /* setup Output buffer */
//     OutputBuffer result_buffer(Result_t);
//     stringstream titles_ss;
//     titles_ss << "locus\talleles\tt_stat\tn\n";
//     result_buffer.extend(titles_ss.str());
//     cout << "Beta Bufer initialized" << endl;

//     /* process rows */
//     SSE_row* sse;
//     XTX_row* xtx;
//     while (true) {
//         try {
//             if (!(sse = (SSE_row*)sse_buffer.get_nextrow())) break;
//             if (!(xtx = (XTX_row*)xtx_buffer.get_nextrow())) break;
//         } catch (ERROR_t& err) {
//             cerr << "ERROR: " << err.msg << endl;
//             continue;
//         }
//         ostringstream ss;
//         ss << sse->getloci() << "\t" << sse->getalleles();
//         try {
//             vector<double> beta;
//             sse->t_stat(*xtx, beta)
//             for (auto b : beta) {
//                 ss << "\t" << b;
//             }
//         } catch (ERROR_t& err) {
//             cerr << "ERROR while processing " << ss.str() << ": " << err.msg
//                  << endl;
//             for (size_t i = 0; i < dim; i++) {
//                 ss << "\tNA";
//             }
//         }
//         delete xtx_row, xty_row;
//     }
//     writebatch(BETA_t, beta_buffer.copy_to_host());
//     cout << "Linear regression beta calculation finished!" << endl;
// }
// delete xtx_row, xty_row;
//     }
//     writebatch(BETA_t, beta_buffer.copy_to_host());
//     cout << "Linear regression beta calculation finished!" << endl;
}

