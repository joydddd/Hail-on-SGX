#include <math.h>

#include <limits>

#include "logistic_regression.h"

/////////////////////////////////////////////////////////////
//////////              Log_row             /////////////////
/////////////////////////////////////////////////////////////

Log_row::Log_row(size_t size, GWAS* _gwas) : Row(size), gwas(_gwas) {
    beta_delta.resize(gwas->dim());
    // 2 is a magic number that helps with SqrMatrix construction, "highest level matrix"
    H = SqrMatrix(gwas->dim(), 2);
    Grad.resize(gwas->dim());
    //b.resize(gwas->dim(), 0);
    // std::cout << b.size() << std::endl;
}

/* fitting */
bool Log_row::fit(std::vector<double>& change, std::vector<double>& old_beta, size_t max_it, double sig) {
    /* intialize beta to 0*/
    init();

    for (int i = 0; i < gwas->dim(); ++i) {
        change[i] = 1;
    }
    size_t it_count = 0;

    //start_timer("while_loop");
    while (it_count < max_it && max(change) > sig) {
        for (int i = 0; i < gwas->dim(); ++i) {
            old_beta[i] = b[i];
        }
        update_beta();
        for (size_t j = 0; j < gwas->dim(); j++) {
            change[j] = abs(b[j] - old_beta[j]);
        }
        it_count++;
    }
    //stop_timer("while_loop");
    if (it_count == max_it)
        return false;
    else {
        fitted = true;
        //start_timer("calculate_standard_error");
        H.INV();
        standard_error = sqrt((*H.t)[0][0]);
        //stop_timer("calculate_standard_error");
        // DEBUG:
        // cout << loci << "\t" << alleles << "\t" << it_count << endl;
        return true;
    }
}

/* output results*/
double Log_row::output_first_beta_element() {
    if (!fitted)
        for (auto &bn : b) bn = nan("");
    return b.front();
}

double Log_row::t_stat() {
    if (!fitted) return nan("");
    return b[0] / standard_error;
}

/* fitting helper functions */

void Log_row::update_beta() {
    // calculate_beta
    H.INV();
    H.t->calculate_beta_delta(Grad, beta_delta);
    for (size_t j = 0; j < gwas->dim(); j++) {
        b[j] += beta_delta[j];
    }
    update_estimate();
}

void Log_row::init() {
    if (gwas->size() != n) throw CombineERROR("row length mismatch");
    if (!b.size()) {
        b.resize(n);
    }
    for (int i = 0; i < n; ++i) {
        b[i] = 0;
    }
    update_estimate();
}

void Log_row::update_estimate() {
    for (int i = 0; i < gwas->dim(); ++i) {
        Grad[i] = 0;
        for (int j = 0; j < gwas->dim(); ++j){
            H[i][j] = 0;
        }
    }
    double y_est;
    size_t client_idx = 0, data_idx = 0;
    for (size_t i = 0; i < n; i++) {
        uint8_t x = data[client_idx][data_idx];
        if (!is_NA(x)) {
            y_est = b[0] * x;
            for (size_t j = 1; j < gwas->dim(); j++)
                y_est += gwas->covariants[j - 1].data[i] * b[j];
            y_est = 1 / ((double)1 + exp(-y_est));

            update_upperH(y_est, x, i);
            update_Grad(y_est, x, i);
        }

        /* update data index */
        data_idx++;
        if (data_idx >= length[client_idx]) {
            data_idx = 0;
            client_idx++;
        }
    }

    /* build lower half of H */
    for (size_t j = 0; j < gwas->dim(); j++) {
        for (size_t k = j + 1; k < gwas->dim(); k++) {
            H[j][k] = H[k][j];
        }
    }
}

void Log_row::update_upperH(double y_est, uint8_t x, size_t i) {
    double y_est_1_y = y_est * (1 - y_est);
    for (size_t j = 0; j < gwas->dim(); j++) {
        for (size_t k = 0; k <= j; k++) {
            double x1 = (j == 0) ? x : gwas->covariants[j - 1].data[i];
            double x2 = (k == 0) ? x : gwas->covariants[k - 1].data[i];
            H[j][k] += x1 * x2 * y_est_1_y;
        }
    }
}

void Log_row::update_Grad(double y_est, uint8_t x, size_t i) {
    double y_delta = gwas->y.data[i] - y_est;
    Grad[0] += y_delta * x;
    for (size_t j = 1; j < gwas->dim(); j++) {
        Grad[j] += y_delta * gwas->covariants[j - 1].data[i];
    }
}
