#ifndef __LOG_REG_H_
#define __LOG_REG_H_

#include "enc_gwas.h"
#include "Matrix.h"
#ifdef NON_OE
#include "enclave_glue.h"
#else
#include "gwas_t.h"
#endif
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
    const Log_gwas *gwas;

    /* model data */
    vector<double> b;
    // vector<double> change;
    vector<double> old_beta;
    vector<double> beta_delta;
    SqrMatrix H;
    // SqrMatrix sub;
    // SqrMatrix cof;
    // SqrMatrix t;
    vector<double> Grad;
    double standard_error;
    bool fitted = false;
    void update_beta();


    void update_estimate();
    inline void update_upperH(double y_est, uint8_t x, size_t i);
    inline void update_Grad(double y_est, uint8_t x, size_t i);
    void init();

   public:
    /* setup */
    Log_row(size_t size, Log_gwas* _gwas);

    /* fitting */
    // return true if converge, return false if explode
    bool fit(std::vector<double>& change, std::vector<double>& old_beta, size_t max_iteration = 25, double sig = 1e-6);

    /* output results */
    vector<double> beta();
    double t_stat();
};

#endif