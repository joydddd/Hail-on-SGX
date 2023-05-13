#include <math.h>

#include <limits>

#include "oblivious_linear_regression.h"

// DEBUG:
#include <iostream>

Oblivious_lin_row::Oblivious_lin_row(size_t _size, GWAS* _gwas, ImputePolicy _impute_policy)
    : Row(_size, _impute_policy), gwas(_gwas), XTX(gwas->dim(), 2), XTX_og(gwas->dim(), 2) {
    beta.resize(gwas->dim());
    XTY.resize(gwas->dim());
    XTY_og.resize(gwas->dim());
    SSE.resize(gwas->dim());

    /* calculate XTX_og, XTY_og */
    for (int i = 0; i < gwas->dim(); i++) {
        XTY_og[i] = 0;
        for (int j = 0; j < gwas->dim(); j++) {
            XTX_og[i][j] = 0;
        }
    }

    for (int i = 0; i < n; ++i){
        double y = gwas->y->data[i];
        for (int j = 1; j < gwas->dim(); ++j) {  // starting from second row
            XTY_og[j] += gwas->covariants[j - 1]->data[i] * y;
            for (int k = 1; k <= j; ++k){
                XTX_og[j][k] += gwas->covariants[j - 1]->data[i] *
                               gwas->covariants[k - 1]->data[i];
            }
        }
    }

    for (int j = 0; j < gwas->dim(); j++){
        for (int k = j + 1; k < gwas->dim(); ++k){
            XTX_og[j][k] = XTX_og[k][j];
        }
    }
}

void Oblivious_lin_row::init() {}

bool Oblivious_lin_row::fit(size_t max_iteration, double sig) {
    for (int i = 0; i < gwas->dim(); i++) {
        beta[i] = 0;
        XTY[i] = XTY_og[i];
        for (int j = 0; j < gwas->dim(); j++) {
            XTX[i][j] = XTX_og[i][j];
        }
    }

    double sum = 0;
    double count = 0;
    uint8_t val;
    uint8_t not_NA_oblivious;
    for (int i = 0 ; i < n; ++i) {
        val = (data[i / 4] >> ((i % 4) * 2)) & 0b11;
        not_NA_oblivious = is_not_NA_oblivious(val);
        sum += val * not_NA_oblivious;
        count += not_NA_oblivious;
    } 

    // TODO: figure out if this is what we want?
    // if count > 0: count = count
    // if count == 0: count = 0
    // avoids divide by 0 issue!
    genotype_average = sum / (count + !count);

    /* calculate XTX & XTY*/
    size_t client_idx = 0, data_idx = 0;
    for (int i = 0; i < n; ++i) {
        double x = (data[i / 4] >> ((i % 4) * 2)) & 0b11;
        not_NA_oblivious = is_not_NA_oblivious(x);
        x = (not_NA_oblivious * x) + (!not_NA_oblivious * genotype_average);
        
        double y = gwas->y->data[i];
        
        XTY[0] += x * y;
        for (int j = 0; j < gwas->dim(); ++j) {
            double x1 = (j == 0) ? x : gwas->covariants[j - 1]->data[i];
            XTX[j][0] += x1 * x;
        }

        /* update data index */
        data_idx++;
        if (data_idx >= length[client_idx]) {
            data_idx = 0;
            client_idx++;
        }
    }

    for (int j = 0; j < gwas->dim(); j++) {
        for (int k = j + 1; k < gwas->dim(); ++k) {
            XTX[j][k] = XTX[k][j];
        }
    }

    /* beta = (XTX)-1 XTY */
    XTX.INV();
    XTX.t->calculate_matrix_times_vec(XTY, beta);

    /* calculate standard error */
    double sse = 0;
    client_idx = 0;
    data_idx = 0;
    for (int i = 0; i < n; ++i) {
        double x = (data[i / 4] >> ((i % 4) * 2)) & 0b11;
        not_NA_oblivious = is_not_NA_oblivious(x);
        x = (not_NA_oblivious * x) + (!not_NA_oblivious * genotype_average);

        double y = gwas->y->data[i];
        double y_est = beta[0] * x;
        for (int j = 1; j < gwas->dim(); j++){
            y_est += gwas->covariants[j - 1]->data[i] * beta[j];
        }
        sse += (y - y_est) * (y - y_est);

        /* update data index */
    }

    sse = sse / (n - gwas->dim() - 1);
    for (int j = 0; j < gwas->dim(); ++j){
        SSE[j] = sse*(*XTX.t)[j][j];
    }
    return true;
}

double Oblivious_lin_row::get_beta() {
    return beta[0];
}

double Oblivious_lin_row::get_standard_error() {
    return sqrt(SSE[0]);
}

double Oblivious_lin_row::get_t_stat() {
    return beta[0] / sqrt(SSE[0]);
}
