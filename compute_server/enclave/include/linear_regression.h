#ifndef __LIN_REG_H_
#define __LIN_REG_H_
/* For linear regression (aggregate on client approach)*/

#include "enc_gwas.h"
#include "Matrix.h"
#ifdef NON_OE
#include "enclave_glue.h"
#else
#include "gwas_t.h"
#endif

class Lin_row : public Row {
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
    Lin_row(size_t size, GWAS* _gwas);

    /* fitting */
    void fit();
    bool fit(std::vector<double>& change, std::vector<double>& old_beta, size_t max_iteration = 20, double sig = 1e-6) { return false; }
    
    /* output results */
    double output_first_beta_element() { return beta[0]; };
    double t_stat();
    int size() { return n; }
    /* reqires boost library. To avoid using boost:
    find t_stat and degree of freedom = n-beta.size()-1 and apply CDF of t
    distribution outside of enclave
    */
};

#endif