#ifndef __OBLIV_LIN_REG_H_
#define __OBLIV_LIN_REG_H_
/* For linear regression (aggregate on client approach)*/

#include "enc_gwas.h"
#include "Matrix.h"
#ifdef NON_OE
#include "enclave_glue.h"
#else
#include "gwas_t.h"
#endif

class Oblivious_lin_row : public Row {
    const GWAS *gwas;

    /* model data */
    std::vector<double> beta; // beta for results
    std::vector<double> SSE;
    SqrMatrix XTX, XTX_og;  // XTX_og contains the part of XTX not dependent on
                            // phenotype, assuming all samples are valid.
    std::vector<double> XTY, XTY_og;

    void init();

   public:
   /* setup */
    Oblivious_lin_row(size_t size, GWAS* _gwas, ImputePolicy _impute_policy);

    /* fitting */
    bool fit(size_t max_iteration = 15, double sig = 1e-6);
    
    /* output results */
    double get_beta();
    double get_t_stat();
    double get_standard_error();

    int size() { return n; }
    /* reqires boost library. To avoid using boost:
    find t_stat and degree of freedom = n-beta.size()-1 and apply CDF of t
    distribution outside of enclave
    */
};

#endif