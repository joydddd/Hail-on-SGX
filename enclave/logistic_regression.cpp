#include <math.h>

#include "gwas.h"
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

GWAS_var::GWAS_var(istream &is) {
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
    if (name() != other.name()) throw CombineERROR("covariant/y name mismatch");
    n += other.n;
    data.reserve(n);
    for (auto x : other.data) {
        data.push_back(x);
    }
}

GWAS_row::GWAS_row(const GWAS_logic &_gwas, string line) : gwas(_gwas) {
    vector<string> parts;
    split_tab(line, parts);
    if (parts.size() < 2) throw ReadtsvERROR(line);
    loci = Loci(parts[0]);
    alleles.read(parts[1]);
    n = parts.size() - 2;
    data.resize(n);
    for (size_t i = 0; i < n; i++) data[i] = read_entry_int(parts[i + 2]);
}

double GWAS_row::estimate_y(size_t i){
    if (data[i] == NA) return NA;
    double y_est;
    y_est = b[0] * data[i];
    for (size_t j = 1; j < gwas.dim(); j++) {
        y_est += gwas.covariants[j - 1].data[i] * b[j];
    }
    y_est = 1 / (1 + exp(-y_est));
    return y_est;
}


void GWAS_row::combine(GWAS_row &other) {
    if (other.gwas.name != gwas.name) throw CombineERROR("gwas mismatch");
    if (other.loci != loci) throw CombineERROR("locus mismatch");
    if (other.alleles != alleles) throw CombineERROR("alleles mismatch");
    n += other.n;
    data.reserve(n);
    for (auto x : other.data) {
        data.push_back(x);
    }
}

void GWAS_row::update_beta() {
    if (gwas.size() != n) throw CombineERROR("row length mismatch");
    if (gwas.dim() != b.size()) throw CombineERROR("gwas dimesion mismatch");
    for (size_t i = 0; i < n; i++) {
        if (data[i] == NA) continue;
        double co;
        double y_est_i = estimate_y(i);
        co = gwas.alpha * (gwas.y.data[i] - y_est_i) * y_est_i *
             (1 - y_est_i);
        b[0] += co * data[i];
        // cerr << co <<"data: "<<data[i] << " i=" << i << endl;
        // if (co*data[i] != 0)
            // cerr << b[0] << " i=" << i << " SE:" << SE() <<endl;
        // for (auto xx : beta()) cerr << "\t" << xx;
        // cerr << endl;
        // cerr << "SE: " << SE() << endl;
        for (size_t j = 1; j < gwas.dim(); j++) {
            b[j] += co * gwas.covariants[j - 1].data[i];
        }
    }
}

SqrMatrix GWAS_row::H() {
    vector<double> y_est_1_y;
    y_est_1_y.resize(n);
    for (size_t i = 0; i < n; i++){
        double y_est = estimate_y(i);
        y_est_1_y[i] = y_est * (1 - y_est);
    }
    vector<vector<double>> H;
    H.resize(gwas.dim());
    for (size_t j = 0; j < gwas.dim(); j++) {
        H[j].resize(gwas.dim());
        for (size_t k = 0; k <= j; k++) {
            H[j][k] = 0;
            for (size_t i = 0; i < n; i++) {
                if (data[i] == NA) continue;
                double y1 =
                    (j == 0) ? gwas.y.data[i] : gwas.covariants[j - 1].data[i];
                double y2 =
                    (k == 0) ? gwas.y.data[i] : gwas.covariants[k - 1].data[i];
                H[j][k] += y1 * y2 * y_est_1_y[i];
            }

        }
    }
    for (size_t j = 0; j < gwas.dim(); j++) {
        for (size_t k = j + 1; k < gwas.dim(); k++) {
            H[j][k] = H[k][j];
        }
    }
    SqrMatrix H_m(H);
    return H_m;
}
double GWAS_row::SE() {
    SqrMatrix H(this->H());
    return sqrt(H.INV()[0][0]);
}