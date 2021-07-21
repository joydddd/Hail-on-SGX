#ifndef GWAS_ENCLAVE_H
#define GWAS_ENCLAVE_H

#define NA -1
#include <vector>

#include "../gwas.h"
#include "../linear_algebra/Matrix.h"

using namespace std;
class CombineERROR {
   public:
    CombineERROR(string _msg) : msg(_msg) {}
    string msg;
};

/* For linear regression (distributed approach)*/

class XTY_row {
   public:
    Alleles alleles;
    Loci loci;
    vector<double> XTY;
    size_t m;

    XTY_row(string str);
    void combine(const XTY_row &other);
    size_t size() const { return m; }
};

class XTX_row {
    Alleles alleles;
    Loci loci;
    SqrMatrix XTX;
    SqrMatrix XTX_1;
    size_t m;

   public:
    XTX_row(string &line);
    void combine(const XTX_row &other);
    void beta(vector<double> &beta, XTY_row &xty);
    size_t size() const { return m; }
    void print() {
        cout << alleles << loci << endl;
        cout << XTX;
    }
    SqrMatrix &INV();
};

class SSE_row {
    Alleles alleles;
    Loci loci;
    int n;  // sample count
    double SSE;

   public:
    SSE_row(string &line);
    void combine(SSE_row &other);
    double t_stat(SqrMatrix &XTX_1, vector<double> &beta);
    double p(SqrMatrix &XTX_1, vector<double> &beta);
    /* reqires boost library. To avoid using boost:
    find t_stat and degree of freedom = n-beta.size()-1 and apply CDF of t
    distribution outside of enclave
    */
};

/* for logistic regression */

class GWAS_var {
    vector<double> data;
    size_t n;
    string name_str;
    friend class GWAS_row;
    friend class GWAS_logic;

   public:
    GWAS_var() : n(0), name_str("NA") {}
    GWAS_var(istream &is);
    GWAS_var(size_t size, int x=1) : data(size, x), n(size), name_str("1"){}
    size_t size() { return n; }
    void combine(GWAS_var &other);
    string name() { return name_str; }
    GWAS_var& operator=(GWAS_var& rhs){
        if (this == &rhs) return *this;
        data = rhs.data;
        n = rhs.n;
        name_str = rhs.name_str;
        return *this;
    }
};

class GWAS_logic {
    vector<GWAS_var> covariants;
    string name;
    GWAS_var y;
    size_t m;      // dimention
    size_t n;      // same size

    void add_y(GWAS_var &_y) {
        if (_y.size() != n) throw CombineERROR("y");
        y = _y;
        name = _y.name() + "_logic_gwas";
    }

   public:
    GWAS_logic(GWAS_var _y) : n(_y.size()), m(1) {
        add_y(_y);
    }

    void add_covariant(GWAS_var &cov) {
        if (cov.size() != n) throw CombineERROR("covariant");
        covariants.push_back(cov);
        m++;
    }
    size_t dim() const { return m; }
    size_t size() const { return n; }
    friend class GWAS_row;
    void print() const;

};  // Gwas class for logic regression

class GWAS_row {
    Loci loci;
    Alleles alleles;
    size_t n;
    vector<double> data;
    vector<double> b;
    vector<double> y_est;
    const GWAS_logic &gwas;
    double estimate_y(size_t i); // estimate y for the ith element
    void update_estimate();
    SqrMatrix H();
    double L();
    vector<double> Grad();

   public:
    GWAS_row(const GWAS_logic &_gwas):gwas(_gwas), n(0){}
    GWAS_row(const GWAS_logic& _gwas, string line);
    void init() {
        b = vector<double>(gwas.dim(), 0);
        update_estimate();
    }  // m for dimension of beta
    size_t size() { return n; }
    void combine(GWAS_row &other);
    void update_beta();
    bool fit(size_t max_iteration = 25, double sig = 0.000001);
    /* return true if converge, return false if explode*/
    vector<double> &beta() { return b; }
    double SE();
    double t_stat() { return b[0] / SE(); }

};

inline size_t split_tab(string &line, vector<string> &parts) {
    parts.clear();
    string part;
    stringstream ss(line);
    while (getline(ss, part, '\t')) {
        parts.push_back(part);
    }
    return parts.size();
}

#endif