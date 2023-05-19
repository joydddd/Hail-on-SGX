#include <math.h>

#include <limits>

#include "linear_regression.h"

// DEBUG:
#include <iostream>

Lin_row::Lin_row(size_t _size, const std::vector<int>& sizes, GWAS* _gwas, ImputePolicy _impute_policy)
    : Row(_size, sizes, _impute_policy), gwas(_gwas), XTX(gwas->dim(), 2), XTX_og(gwas->dim(), 2) {

    impute_average = impute_policy == ImputePolicy::Hail;
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

void Lin_row::init() {}

bool Lin_row::fit(size_t max_iteration, double sig) {
    for (int i = 0; i < gwas->dim(); i++) {
        beta[i] = 0;
        XTY[i] = XTY_og[i];
        for (int j = 0; j < gwas->dim(); j++) {
            XTX[i][j] = XTX_og[i][j];
        }
    }

    if (impute_average) {
        double sum = 0;
        double count = 0;
        uint8_t val;
        for (int i = 0 ; i < n; ++i) {
            val = (data[i / 4] >> ((i % 4) * 2)) & 0b11;
            if (is_not_NA_oblivious(val)) {
                sum += val;
                count++;
            }
        }

        if (count) {
            genotype_average = sum / count;
        }
    }

    /* calculate XTX & XTY*/
    unsigned int data_idx = 0, client_offset = 0, client_idx = 0;
    bool is_NA;
    for (int i = 0; i < n; ++i) {
        //double x = data[i];
        double x = (data[(i + client_offset) / 4] >> (((i + client_offset) % 4) * 2) ) & 0b11;
        is_NA = is_NA_uint8(x);
        x = impute_average && is_NA ? genotype_average : x;
        double y = gwas->y->data[i];

        if (!is_NA) {
            XTY[0] += x * y;
            for (int j = 0; j < gwas->dim(); ++j) {
                double x1 = (j == 0) ? x : gwas->covariants[j - 1]->data[i];
                XTX[j][0] += x1 * x;
            }
        } else { // adjust the part non valid
        // TODO: some optimization here: whether to precalculate Xcov * Y and Xocv * Xcov for each patient
            //std::cout << "???" << std::endl;
            for (int j = 1; j < gwas->dim(); ++j){
                XTY[j] -= gwas->covariants[j - 1]->data[i] * y;
                for (int k = 1; k <= j; ++k){
                    XTX[j][k] -= gwas->covariants[j - 1]->data[i] *
                                 gwas->covariants[k - 1]->data[i];
                }
            }
        }

        /* update data index */
        data_idx++;
        if (data_idx >= client_lengths[client_idx]) {
            data_idx = 0;
            client_offset += (4 - ((1 + i + client_offset) % 4)) % 4; // how many pairs of bits do we need to skip?
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
    client_offset = 0;
    data_idx = 0;
    client_idx = 0;

    for (int i = 0; i < n; ++i) {
        //double x = data[i];
        double x = (data[(i + client_offset) / 4] >> (((i + client_offset) % 4) * 2) ) & 0b11;
        is_NA = is_NA_uint8(x);
        x = impute_average && is_NA ? genotype_average : x;
        double y = gwas->y->data[i];
        if (is_NA) {
            double y_est = beta[0] * x;
            for (int j = 1; j < gwas->dim(); j++){
                y_est += gwas->covariants[j - 1]->data[i] * beta[j];
            }
            sse += (y - y_est) * (y - y_est);
        }

        /* update data index */
        data_idx++;
        if (data_idx >= client_lengths[client_idx]) {
            data_idx = 0;
            client_offset += (4 - ((1 + i + client_offset) % 4)) % 4; // how many pairs of bits do we need to skip?
        }
    }

    sse = sse / (n - gwas->dim() - 1);
    for (int j = 0; j < gwas->dim(); ++j){
        SSE[j] = sse*(*XTX.t)[j][j];
    }
    return true;
}

double Lin_row::get_beta() {
    return beta[0];
}

double Lin_row::get_standard_error() {
    return sqrt(SSE[0]);
}

double Lin_row::get_t_stat() {
    return beta[0] / sqrt(SSE[0]);
}
