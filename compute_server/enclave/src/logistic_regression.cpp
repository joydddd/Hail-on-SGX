#include <math.h>

#include <limits>

#include "logistic_regression.h"

double read_entry_int(std::string &entry) {
    double ans;
    try {
        ans = std::stoi(entry);
    } catch (std::invalid_argument &error) {
        if (entry == "true" || entry == "True") {
            ans = 1;
        } else if (entry == "false" || entry == "False") {
            ans = 0;
        } else if (entry == "NA") {
            ans = nan("");
        } else {
            throw ReadtsvERROR("invalid int entry " + entry);
        }
    }
    return ans;
}

double max(std::vector<double>& vec) {
    double max = -std::numeric_limits<double>::infinity();
    for (auto x : vec) {
        if (x >= max) max = x;
    }
    return max;
}

bool read_entry_bool(std::string& entry) {
    bool ans;
    try {
        int ans_int = stoi(entry);
        if (ans_int == 0) ans = false;
        if (ans_int == 1) ans = true;
    } catch (std::invalid_argument &error) {
        if (entry == "true" || entry == "True") {
            ans = true;
        } else if (entry == "false" || entry == "False") {
            ans = false;
        } else {
            throw ReadtsvERROR("invalid bool entry " + entry);
        }
    }
    return ans;
}

#ifdef DEBUG
void Log_gwas::print() const {
    cout << "y:";
    for (auto xx : y.data) cout << "\t" << xx;
    cout << endl;
}
#endif

void Log_var::read(std::istream &is) {
    std::string line;
    getline(is, line);
    split_delim(line.c_str(), part, parts);
    if (parts.size() != 2) throw ReadtsvERROR("invalid line " + line);
    name_str = line[1];
    while (getline(is, line)) {
        split_delim(line.c_str(), part, parts);
        if (parts.size() != 2) throw ReadtsvERROR("invalid line " + line);
        data.push_back((int)read_entry_bool(parts[1]));
    }
    n = data.size();
}

void Log_var::combine(Log_var &other) {
    if (name() != other.name() && name() != "NA" && other.name() != "NA")
        throw CombineERROR("covariant/y name mismatch");
    n += other.n;
    data.reserve(n);
    for (auto x : other.data) {
        data.push_back(x);
    }
    if (name_str == "NA") name_str = other.name_str;
}

/////////////////////////////////////////////////////////////
//////////              Log_row             /////////////////
/////////////////////////////////////////////////////////////

Log_row::Log_row(size_t size, Log_gwas* _gwas) : Row(size), gwas(_gwas) {
    // change.resize(gwas->dim());
    // old_beta.resize(gwas->dim());
    beta_delta.resize(gwas->dim());
    H = SqrMatrix(gwas->dim(), 2);
    // sub = SqrMatrix(gwas->dim() - 1, false);
    // cof = SqrMatrix(gwas->dim(), false);
    // t = SqrMatrix(gwas->dim(), false);
    Grad.resize(gwas->dim());
    //  = vector<double>(gwas->dim(), 0);
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
