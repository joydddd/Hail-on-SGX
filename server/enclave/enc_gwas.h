#ifndef GWAS_ENCLAVE_H
#define GWAS_ENCLAVE_H

#include "../server_type.h"
#define NA -1
#include <cstring>
#include <deque>
#include <map>
#include <vector>

#include "gwas.h"
#include "linear_algebra/Matrix.h"


    using namespace std;


enum Seal_T { XTX_Seal, BETA_Seal };

/****
 * statics calculation
 */

/* For linear regression (distributed approach)*/

class Row {
   public:
    virtual void read(string &line) = 0;
    virtual void combine(const Row *other) = 0;
    virtual void append_invalid_elts(size_t size) {}
    virtual ~Row() {}
};

class XTY_row : public Row {
   public:
    Alleles alleles;
    Loci loci;
    vector<double> XTY;
    size_t m;

    const Alleles &getalleles() { return alleles; }
    const Loci &getloci() { return loci; }
    void read(string &str);
    void combine(const Row *other);
    size_t size() const { return m; }
};

class XTX_row : public Row {
    Alleles alleles;
    Loci loci;
    SqrMatrix XTX;
    SqrMatrix XTX_1;
    size_t m;

   public:
    const Alleles &getalleles() { return alleles; }
    const Loci &getloci() { return loci; }
    void read(string &line);
    void combine(const Row *other);
    void beta(vector<double> &beta, XTY_row &xty);
    size_t size() const { return m; }
#ifdef DEBUG
    void print() { cout << alleles << loci << endl; }
#endif
    SqrMatrix &INV();
};

class SSE_row : public Row {
    Alleles alleles;
    Loci loci;
    int n;  // sample count
    double SSE;

   public:
    const Alleles &getalleles() { return alleles; }
    const Loci &getloci() { return loci; }
    void read(string &line);
    void combine(const Row *other);
    double t_stat(SqrMatrix &XTX_1, vector<double> &beta);
    // double p(SqrMatrix &XTX_1, vector<double> &beta);
    int size() { return n; }
    /* reqires boost library. To avoid using boost:
    find t_stat and degree of freedom = n-beta.size()-1 and apply CDF of t
    distribution outside of enclave
    */
};

/* for logistic regression */

class Log_var {
    vector<double> data;
    size_t n;
    string name_str;
    friend class Log_row;
    friend class Log_gwas;

   public:
    Log_var() : n(0), name_str("NA") {}
    Log_var(istream &is) { read(is); }
    void read(istream &is);
    Log_var(size_t size, int x = 1) : data(size, x), n(size), name_str("1") {}
    size_t size() { return n; }
    void combine(Log_var &other);
    string name() { return name_str; }
    Log_var &operator=(Log_var &rhs) {
        if (this == &rhs) return *this;
        data = rhs.data;
        n = rhs.n;
        name_str = rhs.name_str;
        return *this;
    }
};

class Log_gwas {
    vector<Log_var> covariants;
    string name;
    Log_var y;
    size_t m;  // dimention
    size_t n;  // same size

   public:
    Log_gwas() : n(0), m(0) {}
    Log_gwas(Log_var _y) : n(_y.size()), m(1) { add_y(_y); }

    void add_y(Log_var &_y) {
        n = _y.size();
        m = 1;
        y = _y;
        name = _y.name() + "_logic_gwas";
    }

    void add_covariant(Log_var &cov) {
        if (cov.size() != n) throw CombineERROR("covariant");
        covariants.push_back(cov);
        m++;
    }
    size_t dim() const { return m; }
    size_t size() const { return n; }
    friend class Log_row;
#ifdef DEBUG
    void print() const;
#endif

};  // Gwas class for logic regression

class Log_row : public Row {
    /* meta data */
    Loci loci;
    Alleles alleles;
    size_t n;
    const Log_gwas &gwas;

    /* data */
    vector<double> data;

    /* model data */
    vector<double> b;
    vector<double> y_est;
    double standard_error;
    bool fitted = false;

    double estimate_y(size_t i);  // estimate y for the ith element
    void update_estimate();
    void update_beta();
    SqrMatrix H();
    double L();
    vector<double> Grad();
    void init() {
        b = vector<double>(gwas.dim(), 0);
        update_estimate();
    }  // m for dimension of beta

   public:
    /* setup */
    Log_row(const Log_gwas &_gwas) : gwas(_gwas), n(0) {}
    void read(string &line);
    void combine(const Row *other);
    void append_invalid_elts(size_t _n);

    /* resturn metadata */
    Loci getloci() { return loci; }
    Alleles getallels() { return alleles; }
    size_t size() { return n; }

    /* fitting */
    // return true if converge, return false if explode
    bool fit(size_t max_iteration = 25, double sig = 0.000001);

    /* output results */
    vector<double> beta();
    double t_stat();
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