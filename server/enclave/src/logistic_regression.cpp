#include <math.h>

#include <limits>

#include "logistic_regression.h"
using namespace std;

double read_entry_int(string &entry) {
    double ans;
    try {
        ans = stoi(entry);
    } catch (invalid_argument &error) {
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

double max(vector<double> &vec) {
    double max = -numeric_limits<double>::infinity();
    for (auto x : vec) {
        if (x >= max) max = x;
    }
    return max;
}

bool read_entry_bool(string &entry) {
    bool ans;
    try {
        int ans_int = stoi(entry);
        if (ans_int == 0) ans = false;
        if (ans_int == 1) ans = true;
    } catch (invalid_argument &error) {
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

void Log_var::read(istream &is) {
    string line;
    vector<string> parts;
    getline(is, line);
    split_tab(line, parts);
    if (parts.size() != 2) throw ReadtsvERROR("invalid line " + line);
    name_str = line[1];
    while (getline(is, line)) {
        split_tab(line, parts);
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

/* fitting */
bool Log_row::fit(const Log_gwas* _gwas, size_t max_it, double sig) {
    gwas = _gwas;
    /* intialize beta to 0*/
    init();

    vector<double> change(gwas->dim(), 1);
    size_t it_count = 0;
    while (it_count < max_it && max(change) > sig) {
        vector<double> old_beta = b;
        update_beta();
        for (size_t j = 0; j < gwas->dim(); j++) {
            change[j] = abs(b[j] - old_beta[j]);
        }
        it_count++;
    }
    if (it_count == max_it)
        return false;
    else {
        fitted = true;
        standard_error = sqrt(H.INV()[0][0]);
        // DEBUG:
        // cout << loci << "\t" << alleles << "\t" << it_count << endl;
        return true;
    }
}

/* output results*/
vector<double> Log_row::beta() {
    if (!fitted)
        for (auto &bn : b) bn = nan("");
    return b;
}

double Log_row::t_stat() {
    if (!fitted) return nan("");
    return b[0] / standard_error;
}

/* fitting helper functions */

void Log_row::update_beta() {
    vector<double> beta_delta = H.INV() * Grad;
    for (size_t j = 0; j < gwas->dim(); j++) {
        b[j] += beta_delta[j];
    }
    update_estimate();
}

void Log_row::init() {
    if (gwas->size() != n) throw CombineERROR("row length mismatch");
    b = vector<double>(gwas->dim(), 0);
    update_estimate();
}

void Log_row::update_estimate() {
    H = SqrMatrix(gwas->dim());
    Grad = vector<double>(gwas->dim(), 0);
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
