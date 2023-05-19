#include <math.h>

#include <limits>

#include "oblivious_logistic_regression.h"
#include "gwas.h"

/////////////////////////////////////////////////////////////
//////////              Oblivious_log_row             /////////////////
/////////////////////////////////////////////////////////////

std::vector<int> ababa;

Oblivious_log_row::Oblivious_log_row(size_t _size, GWAS* _gwas, ImputePolicy _impute_policy) : Row(_size, ababa, _impute_policy), gwas(_gwas) {
    beta_delta.resize(gwas->dim());
    // 2 is a magic number that helps with SqrMatrix construction, "highest level matrix"
    H = SqrMatrix(gwas->dim(), 2);
    Grad.resize(gwas->dim());
}

/* fitting */
bool Oblivious_log_row::fit(size_t max_it, double sig) {
    /* intialize beta to 0*/
    init();
    it_count = 0;

    while (it_count < max_it) {
        update_beta();
        it_count++;
    }

    fitted = true;
    H.INV();
    standard_error = sqrt((*H.t)[0][0]);
    return true;
}

/* output results*/
double Oblivious_log_row::get_beta() {
    if (!fitted)
        for (auto &bn : b) bn = nan("");
    return b.front();
}

double Oblivious_log_row::get_t_stat() {
    if (!fitted) return nan("");
    return b[0] / standard_error;
}

double Oblivious_log_row::get_standard_error() {
    if (!fitted) return nan("");
    return standard_error;
}

/* fitting helper functions */

void Oblivious_log_row::update_beta() {
    // calculate_beta
    H.INV();
    H.t->calculate_beta_delta(Grad, beta_delta);
    for (size_t i = 0; i < gwas->dim(); i++) {
        b[i] += beta_delta[i];
        // take abs after adding beta delta so that we can determine if we have passed tolerance
        beta_delta[i] = abs(beta_delta[i]);
    }

    update_estimate();
}

void Oblivious_log_row::init() {
    if (gwas->size() != n) throw CombineERROR("row length mismatch");
    if (b.size() != n) {
        b.resize(n);
    }
    for (int i = 0; i < n; ++i) {
        b[i] = 0;
    }
    for (int i = 0; i < gwas->dim(); ++i) {
        beta_delta[i] = 1;
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

    update_estimate();
}

void Oblivious_log_row::update_estimate() {
    for (int i = 0; i < gwas->dim(); ++i) {
        Grad[i] = 0;
        for (int j = 0; j < gwas->dim(); ++j){
            H[i][j] = 0;
        }
    }
    double y_est;
    size_t client_idx = 0, data_idx = 0;
    double x;
    uint8_t not_NA_oblivious;
    for (size_t i = 0; i < n; i++) {
        double x = (data[i / 4] >> ((i % 4) * 2)) & 0b11;
        not_NA_oblivious = is_not_NA_oblivious(x);
        x = (not_NA_oblivious * x) + (!not_NA_oblivious * genotype_average);
        y_est = b[0] * x;
        for (size_t j = 1; j < gwas->dim(); j++)
            y_est += gwas->covariants[j - 1]->data[i] * b[j];
        y_est = 1 / ((double)1 + exp(-y_est));

        update_upperH(y_est, x, i);
        update_Grad(y_est, x, i);
        /* update data index */
        // data_idx++;
        // if (data_idx >= length[client_idx]) {
        //     data_idx = 0;
        //     client_idx++;
        // }
    }
    /* build lower half of H */
    for (size_t j = 0; j < gwas->dim(); j++) {
        for (size_t k = j + 1; k < gwas->dim(); k++) {
            H[j][k] = H[k][j];
        }
    }
}

void Oblivious_log_row::update_upperH(double y_est, uint8_t x, size_t i) {
    double y_est_1_y = y_est * (1 - y_est);
    for (size_t j = 0; j < gwas->dim(); j++) {
        for (size_t k = 0; k <= j; k++) {
            double x1 = (j == 0) ? x : gwas->covariants[j - 1]->data[i];
            double x2 = (k == 0) ? x : gwas->covariants[k - 1]->data[i];
            H[j][k] += x1 * x2 * y_est_1_y;
        }
    }
}

void Oblivious_log_row::update_Grad(double y_est, uint8_t x, size_t i) {
    double y_delta = gwas->y->data[i] - y_est;
    Grad[0] += y_delta * x;
    for (size_t j = 1; j < gwas->dim(); j++) {
        Grad[j] += y_delta * gwas->covariants[j - 1]->data[i];
    }
}
