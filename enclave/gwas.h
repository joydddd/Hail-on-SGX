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
    double p(SqrMatrix &XTX_1, vector<double> &beta);
};

class GWAS_var {
    vector<int> data;
    size_t n;
    string name;
    friend class GWAS_logic;

   public:
    GWAS_var();
    size_t size() { return n; }
    void combine(GWAS_var &other);
};

class GWAS_row {
    Loci loci;
    Alleles alleles;
    size_t n;
    vector<int> data;
    friend class GWAS_logic;

   public:
    GWAS_row();
    GWAS_row(string line);
    size_t size() { return n; }
    void combine(GWAS_row &other);
};

class GWAS_logic {
    vector<GWAS_var> covariants;
    GWAS_var y;
    vector<double> beta;
    vector<double> est_without_row;
    size_t m;  // dimention
    size_t n;  // same size
    void
    update_est();  // update estimate intermediate result while updating beta
    void estimate(vector<double> &y_est, GWAS_row &row);

   public:
    size_t dim() { return m; }
    size_t size() { return n; }
    SqrMatrix H_1(GWAS_row &row);  // return reverse of Hessian matrix
    vector<double> Delta(GWAS_row &row);

};  // Gwas class for logic regression

#endif