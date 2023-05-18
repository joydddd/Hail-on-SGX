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
    Log_row(size_t _size, const std::vector<int>& sizes, GWAS* _gwas, ImputePolicy _impute_policy);

    /* fitting */
    bool fit(size_t max_iteration = 15, double sig = 1e-6);

    /* output results */
    double get_beta();
    double get_t_stat();
    double get_standard_error();
};

#endif