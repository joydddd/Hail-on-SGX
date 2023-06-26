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
    std::vector<double> y_ests;
    double standard_error;
    bool fitted = false;
    void update_beta();


    void update_estimate();
    inline void update_upperH_and_Grad(double y_est, double x, const std::vector<double>& patient_pnc);
    inline void update_Grad(double y_est, uint8_t x, int i);
    void init();

   public:
    /* setup */
    Log_row(int _size, const std::vector<int>& sizes, GWAS* _gwas, ImputePolicy _impute_policy);

    /* fitting */
    bool fit(int thread_id = -1, int max_iteration = 15, double sig = 1e-6);

    /* output results */
    double get_beta();
    double get_t_stat();
    double get_standard_error();
};

#endif