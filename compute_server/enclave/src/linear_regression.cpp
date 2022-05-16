// #include <boost/math/distributions/students_t.hpp>
#include <math.h>

#include <limits>
// using namespace boost::math;

#include "linear_regression.h"

// DEBUG:
#include <iostream>

Lin_row::Lin_row(size_t _size, GWAS* _gwas)
    : Row(_size), gwas(_gwas), XTX(gwas->dim(), 2), XTX_og(gwas->dim(), 2) {
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
        double y = gwas->y.data[i];
        for (int j = 1; j < gwas->dim(); ++j) {  // starting from second row
            XTY_og[j] += gwas->covariants[j - 1].data[i] * y;
            for (int k = 1; k <= j; ++k){
                XTX_og[j][k] += gwas->covariants[j - 1].data[i] *
                               gwas->covariants[k - 1].data[i];
            }
        }
    }

    for (int j = 0; j < gwas->dim(); j++){
        for (int k = j + 1; k < gwas->dim(); ++k){
            XTX_og[j][k] = XTX_og[k][j];
        }
    }
}

void Lin_row::init() {
}

void Lin_row::fit() {
    for (int i = 0; i < gwas->dim(); i++) {
        beta[i] = 0;
        XTY[i] = XTY_og[i];
        for (int j = 0; j < gwas->dim(); j++) {
            XTX[i][j] = XTX_og[i][j];
        }
    }
    /* calculate XTX & XTY*/
    size_t client_idx = 0, data_idx = 0;
    for (int i = 0; i < n; ++i) {
        uint8_t x = data[client_idx][data_idx];
        double y = gwas->y.data[i];
        if (!is_NA(x)) {
            XTY[0] += x * y;
            for (int j = 0; j < gwas->dim(); ++j) {
                double x1 = (j == 0) ? x : gwas->covariants[j - 1].data[i];
                XTX[j][0] += x1 * x;
            }
        } else { // adjust the part non valid
        // TODO: some optimization here: whether to precalculate Xcov * Y and Xocv * Xcov for each patient
            for (int j = 1; j < gwas->dim(); ++j){
                XTY[j] -= gwas->covariants[j - 1].data[i] * y;
                for (int k = 1; k <= j; ++k){
                    XTX[j][k] -= gwas->covariants[j - 1].data[i] *
                                 gwas->covariants[k - 1].data[i];
                }
            }
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
        uint8_t x = data[client_idx][data_idx];
        double y = gwas->y.data[i];
        if (!is_NA(x)) {
            double y_est = beta[0] * x;
            for (int j = 1; j < gwas->dim(); j++){
                y_est += gwas->covariants[j - 1].data[i] * beta[j];
            }
            sse += (y - y_est) * (y - y_est);
        }

        /* update data index */
        data_idx++;
        if (data_idx >= length[client_idx]) {
            data_idx = 0;
            client_idx++;
        }
    }

    sse = sse / (n - gwas->dim() - 1);
    for (int j = 0; j < gwas->dim(); ++j){
        SSE[j] = sse*(*XTX.t)[j][j];
    } 
}

double Lin_row::t_stat() {
    return beta[0] / sqrt(SSE[0]);
}
