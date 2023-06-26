#include <cmath>

#include <limits>

#include "linear_regression.h"

// DEBUG:
#include <iostream>

Lin_row::Lin_row(int _size, const std::vector<int>& sizes, GWAS* _gwas, ImputePolicy _impute_policy, int thread_id)
    : Row(_size, sizes, _gwas->dim(), _impute_policy), XTX(num_dimensions, 2) {

    //size_of_thread_buffer = gwas->dim();

    impute_average = impute_policy == ImputePolicy::Hail;
    int offset = thread_id * get_padded_buffer_len(num_dimensions);

    double *XTY_og = XTY_og_g + offset;
    
    XTX_og_list[offset] = new double*[num_dimensions];
    for (int i = 0; i < num_dimensions; ++i) {
        XTX_og_list[offset][i] = new double[num_dimensions];
    }

    double **XTX_og = XTX_og_list[offset];

    /* calculate XTX_og, XTY_og */
    for (int i = 0; i < num_dimensions; i++) {
        XTY_og[i] = 0;
        for (int j = 0; j < num_dimensions; j++) {
            XTX_og[i][j] = 0;
        }
    }

    for (int i = 0; i < n; ++i){
        const std::vector<double>& patient_pnc = gwas->phenotype_and_covars.data[i];
        double y = patient_pnc[0];
        for (int j = 1; j < num_dimensions; ++j) {  // starting from second row
            XTY_og[j] += patient_pnc[j] * y;
            for (int k = 1; k <= j; ++k){
                XTX_og[j][k] += patient_pnc[j] * patient_pnc[k];
            }
        }
    }

    for (int j = 0; j < num_dimensions; j++){
        for (int k = j + 1; k < num_dimensions; ++k){
            XTX_og[j][k] = XTX_og[k][j];
        }
    }
}

void Lin_row::init() {}

bool Lin_row::fit(int thread_id, int max_iteration, double sig) {
    int offset = thread_id * get_padded_buffer_len(num_dimensions);
    double *beta = beta_g + offset;
    double *XTY = XTY_g + offset;
    double *XTY_og = XTY_og_g + offset;
    double **XTX_og = XTX_og_list[offset];

    for (int i = 0; i < num_dimensions; i++) {
        beta[i] = 0;
        XTY[i] = XTY_og[i];
        for (int j = 0; j < num_dimensions; j++) {
            XTX.assign(i, j, XTX_og[i][j]);
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
        const std::vector<double>& patient_pnc = gwas->phenotype_and_covars.data[i];

        double x = (data[(i + client_offset) / 4] >> (((i + client_offset) % 4) * 2) ) & 0b11;
        is_NA = is_NA_uint8(x);
        x = impute_average && is_NA ? genotype_average : x;
        double y = patient_pnc[0];

        if (!is_NA) {
            XTY[0] += x * y;
            for (int j = 0; j < num_dimensions; ++j) {
                double x1 = (j == 0) ? x : patient_pnc[j];
                XTX.plus_equals(j, 0, x1 * x);
            }
        } else { // adjust the part non valid
        // TODO: some optimization here: whether to precalculate Xcov * Y and Xocv * Xcov for each patient
            //std::cout << "???" << std::endl;
            for (int j = 1; j < num_dimensions; ++j){
                XTY[j] -= patient_pnc[j] * y;
                for (int k = 1; k <= j; ++k){
                    XTX.minus_equals(j, k, patient_pnc[j] * patient_pnc[k]);
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

    for (int j = 0; j < num_dimensions; j++) {
        for (int k = j + 1; k < num_dimensions; ++k) {
            XTX.inner_assign(j, k, k, j);
        }
    }

    /* beta = (XTX)-1 XTY */
    XTX.INV();
    XTX.calculate_t_matrix_times_vec(XTY, beta);

    /* calculate standard error */
    double sse = 0;
    client_offset = 0;
    data_idx = 0;
    client_idx = 0;

    for (int i = 0; i < n; ++i) {
        const std::vector<double>& patient_pnc = gwas->phenotype_and_covars.data[i];

        double x = (data[(i + client_offset) / 4] >> (((i + client_offset) % 4) * 2) ) & 0b11;
        is_NA = is_NA_uint8(x);
        x = impute_average && is_NA ? genotype_average : x;
        double y = patient_pnc[0];
        if (is_NA) {
            double y_est = beta[0] * x;
            for (int j = 1; j < num_dimensions; j++){
                y_est += patient_pnc[j] * beta[j];
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

    sse = sse / (n - num_dimensions - 1);
    // for (int j = 0; j < num_dimensions; ++j){
    //     SSE[j] = sse * (*XTX.t)[j][j];
    // }

    //beta_ans = beta[0];
    //sse_ans_list[thread_id] = std::sqrt(sse * XTX.t->at(0, 0));

    // overwrite b1 with the standard error, helps with false sharing issue... if we need
    // to report other betas in the future we need to change this!
    beta[1] = std::sqrt(sse * XTX.t[0][0]);
    

    return true;
}

double Lin_row::get_beta(int thread_id) {
    return (beta_g + (thread_id * get_padded_buffer_len(num_dimensions)))[0];//return beta[0];
}

double Lin_row::get_standard_error(int thread_id) {
    return (beta_g + (thread_id * get_padded_buffer_len(num_dimensions)))[1];//return std::sqrt(SSE[0]);
}

double Lin_row::get_t_stat(int thread_id) {
    return (beta_g + (thread_id * get_padded_buffer_len(num_dimensions)))[0] / (beta_g + (thread_id * get_padded_buffer_len(num_dimensions)))[1];//return beta[0] / std::sqrt(SSE[0]);
}
