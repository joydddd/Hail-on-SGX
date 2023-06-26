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

// extern double beta_ans;
// extern double sse_ans;
extern GWAS *gwas;
extern double **beta_list;
extern double **XTY_list;

extern double *beta_g;
extern double *XTY_g;
extern double *XTY_og_g;
extern double ***XTX_og_list;

class Lin_row : public Row {
    //const GWAS *gwas;

    /* model data */
    //std::vector<double> beta; // beta for results
    //std::vector<double> SSE;
    //std::vector< std::vector<double> > XTX_og;
    SqrMatrix XTX;  // XTX_og contains the part of XTX not dependent on
                            // phenotype, assuming all samples are valid.

    int tb;

    void init();

   public:
   /* setup */
    Lin_row(int size, const std::vector<int>& sizes, GWAS* _gwas, ImputePolicy _impute_policy, int thread_id);

    /* fitting */
    bool fit(int thread_id = -1, int max_iteration = 15, double sig = 1e-6);
    
    /* output results */
    double get_beta(int thread_id);
    double get_t_stat(int thread_id);
    double get_standard_error(int thread_id);

    int size() { return n; }
    /* reqires boost library. To avoid using boost:
    find t_stat and degree of freedom = n-beta.size()-1 and apply CDF of t
    distribution outside of enclave
    */
};

#endif