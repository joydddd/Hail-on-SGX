#include "enc_gwas.h"
/* For linear regression (aggregate on client approach)*/

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
