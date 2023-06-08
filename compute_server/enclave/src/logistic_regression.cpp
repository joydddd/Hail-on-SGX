#include <math.h>

#include <limits>

#include "logistic_regression.h"
#include "gwas.h"

/////////////////////////////////////////////////////////////
//////////              Log_row             /////////////////
/////////////////////////////////////////////////////////////

Log_row::Log_row(int _size, const std::vector<int>& sizes, GWAS* _gwas, ImputePolicy _impute_policy) : 
    Row(_size, sizes, _gwas->dim(), _impute_policy), gwas(_gwas) {

    beta_delta.resize(num_dimensions);
    // 2 is a magic number that helps with SqrMatrix construction, "highest level matrix"
    H = SqrMatrix(num_dimensions, 2);
    Grad.resize(num_dimensions);

    if (gwas->size() != n) throw CombineERROR("row length mismatch");
    if (b.size() != n) {
        b.resize(n);
    }
}

/* fitting */
bool Log_row::fit(int max_it, double sig) {
    /* intialize beta to 0*/
    init();
    it_count = 0;

    while (it_count < max_it && bd_max(beta_delta) >= sig) {
        update_beta();
        it_count++;
        break;
    }

    if (it_count == max_it)
        return false;
    else {
        fitted = true;
        H.INV();
        standard_error = sqrt((*H.t)[0][0]);
        return true;
    }
}

/* output results*/
double Log_row::get_beta() {
    if (!fitted)
        for (double &bn : b) bn = nan("");
    return b.front();
}

double Log_row::get_t_stat() {
    if (!fitted) return nan("");
    return b.front() / standard_error;
}

double Log_row::get_standard_error() {
    if (!fitted) return nan("");
    return standard_error;
}

/* fitting helper functions */

void Log_row::update_beta() {
    // calculate_beta
    H.INV();
    H.t->calculate_beta_delta(Grad, beta_delta);
    for (int i = 0; i < num_dimensions; i++) {
        b[i] += beta_delta[i];
        // take abs after adding beta delta so that we can determine if we have passed tolerance
        beta_delta[i] = abs(beta_delta[i]);
    }

    update_estimate();
}

void Log_row::init() {
    for (int i = 0; i < n; ++i) {
        b[i] = 0;
    }
    for (int i = 0; i < num_dimensions; ++i) {
        beta_delta[i] = 1;
    }

    if (impute_average) {
        double sum = 0;
        double count = 0;
        uint8_t val;
        for (int i = 0 ; i < n; ++i) {
            val = (data[i / 4] >> ((i % 4) * 2)) & 0b11;
            if (is_NA_uint8(val)) {
                sum += val;
                count++;
            }
        }

        if (count) {
            genotype_average = sum / count;
        }
    }

    update_estimate();
}

void Log_row::update_estimate() {
    for (int i = 0; i < num_dimensions; ++i) {
        Grad[i] = 0;
        for (int j = 0; j < num_dimensions; ++j){
            H[i][j] = 0;
        }
    }
    double y_est;
    unsigned int data_idx = 0, client_offset = 0, client_idx = 0;
    double x;
    bool is_NA;
    for (int i = 0; i < n; i++) {
        double x = (data[(i + client_offset) / 4] >> (((i + client_offset) % 4) * 2) ) & 0b11;
        is_NA = is_NA_uint8(x);
        x = impute_average && is_NA ? genotype_average : x;
        if (!is_NA) {
            const std::vector<double>& patient_pnc = gwas->phenotype_and_covars.data[i];

            y_est = b[0] * x;
            for (int j = 1; j < num_dimensions; j++) {
                y_est += patient_pnc[j] * b[j];
            }
            y_est = 1 / (1 + exp(-y_est));

            update_upperH_and_Grad(y_est, x, patient_pnc);
            //update_Grad(y_est, x, i);
        }
        /* update data index */
        data_idx++;
        if (data_idx >= client_lengths[client_idx]) {
            data_idx = 0;
            client_idx++;
            client_offset += (4 - ((1 + i + client_offset) % 4)) % 4; // how many pairs of bits do we need to skip?
        }
    }
    /* build lower half of H */
    for (int j = 0; j < num_dimensions; j++) {
        for (int k = j + 1; k < num_dimensions; k++) {
            H[j][k] = H[k][j];
        }
    }
}

void Log_row::update_upperH_and_Grad(double y_est, double x, const std::vector<double>& patient_pnc) {
    double y_est_1_y = y_est * (1 - y_est);
    double y_delta = patient_pnc[0] - y_est;
    Grad[0] += y_delta * x;
    for (int j = 0; j < num_dimensions; j++) {
        if (j > 0) {
            Grad[j] += y_delta * patient_pnc[j];
        }
        for (int k = 0; k <= j; k++) {
            double x1 = (j == 0) ? x : patient_pnc[j];
            double x2 = (k == 0) ? x : patient_pnc[k];
            H[j][k] += x1 * x2 * y_est_1_y;
        }
    }
}

