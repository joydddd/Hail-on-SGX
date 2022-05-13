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

class Log_row : public Row {
    const GWAS *gwas;

    /* model data */
    std::vector<double> b;
    // vector<double> change;
    std::vector<double> old_beta;
    std::vector<double> beta_delta;
    SqrMatrix H;
    std::vector<double> Grad;
    double standard_error;
    bool fitted = false;
    void update_beta();


    void update_estimate();
    inline void update_upperH(double y_est, uint8_t x, size_t i);
    inline void update_Grad(double y_est, uint8_t x, size_t i);
    void init();

   public:
    /* setup */
    Log_row(size_t size, GWAS* _gwas);

    /* fitting */
    // return true if converge, return false if explode
    bool fit(std::vector<double>& change, std::vector<double>& old_beta, size_t max_iteration = 20, double sig = 1e-6);

    /* output results */
    double output_first_beta_element();
    double t_stat();
};

#endif