#include <math.h>
#include <limits>

#include "enc_gwas.h"
using namespace std;

int read_entry_int(string &entry) {
    int ans;
    try {
        ans = stoi(entry);
    } catch (invalid_argument &error) {
        if (entry == "true" || entry == "True") {
            ans = 1;
        } else if (entry == "false" || entry == "False") {
            ans = 0;
        } else if (entry == "NA") {
            ans = NA;
        } else {
            throw ReadtsvERROR("invalid int entry " + entry);
        }
    }
    return ans;
}

double max(vector<double> &vec){
    double max = -numeric_limits<double>::infinity();
    for (auto x:vec){
        if (x > max) max = x;
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

void GWAS_logic::print() const {
    cout << "y:";
    for (auto xx : y.data) cout << "\t" << xx;
    cout << endl;
}

void GWAS_var::read(istream &is) {
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

void GWAS_var::combine(GWAS_var &other) {
    if (name() != other.name() && name() != "NA" && other.name() != "NA")
        throw CombineERROR("covariant/y name mismatch");
    n += other.n;
    data.reserve(n);
    for (auto x : other.data) {
        data.push_back(x);
    }
    if (name_str == "NA") name_str = other.name_str;
}

void GWAS_row::read(string &line){
    vector<string> parts;
    split_tab(line, parts);
    if (parts.size() < 2) throw ReadtsvERROR(line);
    loci = Loci(parts[0]);
    alleles.read(parts[1]);
    n = parts.size() - 2;
    data.resize(n);
    for (size_t i = 0; i < n; i++) data[i] = read_entry_int(parts[i + 2]);
}

double GWAS_row::estimate_y(size_t i) {
    if (data[i] == NA) return NA;
    double y_est;
    y_est = b[0] * data[i];
    for (size_t j = 1; j < gwas.dim(); j++) {
        y_est += gwas.covariants[j - 1].data[i] * b[j];
    }
    y_est = 1 / ((double)1 + exp(-y_est));
    return y_est;
}

void GWAS_row::update_estimate() {
    y_est.clear();
    y_est.resize(n);
    for (size_t i = 0; i < n; i++) y_est[i] = estimate_y(i);
}

SqrMatrix GWAS_row::H() {
    vector<double> y_est_1_y;
    y_est_1_y.resize(n);
    for (size_t i = 0; i < n; i++) {
        y_est_1_y[i] = y_est[i] * (1 - y_est[i]);
    }
    vector<vector<double>> H_vec;
    H_vec.resize(gwas.dim());
    for (size_t j = 0; j < gwas.dim(); j++) {
        H_vec[j].resize(gwas.dim());
        for (size_t k = 0; k <= j; k++) {
            H_vec[j][k] = 0;
            for (size_t i = 0; i < n; i++) {
                if (data[i] == NA) continue;
                double x1 = (j == 0) ? data[i] : gwas.covariants[j - 1].data[i];
                double x2 = (k == 0) ? data[i] : gwas.covariants[k - 1].data[i];
                H_vec[j][k] += x1 * x2 * y_est_1_y[i];
            }
        }
    }
    for (size_t j = 0; j < gwas.dim(); j++) {
        for (size_t k = j + 1; k < gwas.dim(); k++) {
            H_vec[j][k] = H_vec[k][j];
        }
    }
    return SqrMatrix(H_vec);
}

double GWAS_row::L() {
    double L = 0;
    for (size_t i = 0; i < n; i++) {
        if (data[i] == NA) continue;
        L += log(y_est[i]) * gwas.y.data[i] +
             (1 - gwas.y.data[i]) * log(1 - y_est[i]);
    }
    return L;
}

vector<double> GWAS_row::Grad() {
    vector<double> D(gwas.dim(), 0);
    for (size_t i = 0; i < n; i++) {
        if (data[i] == NA) continue;
        double y_delta = gwas.y.data[i] - y_est[i];
        D[0] += y_delta * data[i];
        for (size_t j = 1; j < gwas.dim(); j++) {
            D[j] += y_delta * gwas.covariants[j - 1].data[i];
        }
    }
    return D;
}
void GWAS_row::append_invalid_elts(size_t _n){
    n += _n;
    data.reserve(n);
    for (size_t i = 0; i < _n; i++){
        data.push_back(NA);
    }
}

void GWAS_row::combine(const Row *_other) {
    const GWAS_row *t = (const GWAS_row *)_other;
    const GWAS_row &other(*t);
    if (other.gwas.name != gwas.name) throw CombineERROR("gwas mismatch");
    if (other.loci != loci && loci != Loci() && other.loci != Loci()) throw CombineERROR("locus mismatch");
    if (other.alleles != alleles && alleles != Alleles() && other.alleles != Alleles() ) throw CombineERROR("alleles mismatch");
    n += other.n;
    data.reserve(n);
    for (auto x : other.data) {
        data.push_back(x);
    }
}

void GWAS_row::update_beta() {
    if (gwas.size() != n) throw CombineERROR("row length mismatch");
    if (gwas.dim() != b.size()) throw CombineERROR("gwas dimesion mismatch");
    double change = 10;
    size_t it_count = 0;
    vector<double> D(Grad());
    vector<double> beta_delta = H().INV() * D;
    for (size_t j = 0; j < gwas.dim(); j++) {
        b[j] += beta_delta[j];
    }
    // cerr << "beta: ";
    // for (double xx : b) {
    //     cerr << "\t" << xx;
    // }
    // cerr << endl;
    update_estimate();
}

bool GWAS_row::fit(size_t max_it, double sig) { 
    vector<double> change(gwas.dim(), 1);
    size_t it_count = 0;
    while(it_count < max_it && max(change) > sig){
        vector<double> old_beta = b;
        update_beta();
        for (size_t j = 0; j < gwas.dim(); j++) {
            change[j] = abs(b[j] - old_beta[j]);
        }
        it_count++;
    }
    if (it_count == max_it) return false;
    else
        return true;
}

double GWAS_row::SE() { return sqrt(H().INV()[0][0]); }