#ifndef GWAS_ENCLAVE_H
#define GWAS_ENCLAVE_H

#include "type.h"
#define NA -1
#include <cstring>
#include <deque>
#include <vector>
#include <map>

#include "../gwas.h"
#include "../linear_algebra/Matrix.h"

using namespace std;
class CombineERROR : public ERROR_t {
    using ERROR_t::ERROR_t;
};

/****
 * statics calculation
 */

/* For linear regression (distributed approach)*/

class Row {
   public:
    virtual void read(string &line) = 0;
    virtual void combine(const Row *other) = 0;
    virtual void append_invalid_elts(size_t size){}
};

class XTY_row : public Row {
   public:
    Alleles alleles;
    Loci loci;
    vector<double> XTY;
    size_t m;

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
    void read(string &line);
    void combine(const Row *other);
    void beta(vector<double> &beta, XTY_row &xty);
    size_t size() const { return m; }
    void print() {
        cout << alleles << loci << endl;
        cout << XTX;
    }
    SqrMatrix &INV();
};

class SSE_row : public Row {
    Alleles alleles;
    Loci loci;
    int n;  // sample count
    double SSE;

   public:
    void read(string &line);
    void combine(const Row *other);
    double t_stat(SqrMatrix &XTX_1, vector<double> &beta);
    double p(SqrMatrix &XTX_1, vector<double> &beta);
    /* reqires boost library. To avoid using boost:
    find t_stat and degree of freedom = n-beta.size()-1 and apply CDF of t
    distribution outside of enclave
    */
};

/* for logistic regression */

class GWAS_var {
    vector<double> data;
    size_t n;
    string name_str;
    friend class GWAS_row;
    friend class GWAS_logic;

   public:
    GWAS_var() : n(0), name_str("NA") {}
    GWAS_var(istream &is) { read(is); }
    void read(istream &is);
    GWAS_var(size_t size, int x = 1) : data(size, x), n(size), name_str("1") {}
    size_t size() { return n; }
    void combine(GWAS_var &other);
    string name() { return name_str; }
    GWAS_var &operator=(GWAS_var &rhs) {
        if (this == &rhs) return *this;
        data = rhs.data;
        n = rhs.n;
        name_str = rhs.name_str;
        return *this;
    }
};

class GWAS_logic {
    vector<GWAS_var> covariants;
    string name;
    GWAS_var y;
    size_t m;  // dimention
    size_t n;  // same size

    void add_y(GWAS_var &_y) {
        if (_y.size() != n) throw CombineERROR("y");
        y = _y;
        name = _y.name() + "_logic_gwas";
    }

   public:
    GWAS_logic() : n(0), m(0) {}
    GWAS_logic(GWAS_var _y) : n(_y.size()), m(1) { add_y(_y); }

    void add_covariant(GWAS_var &cov) {
        if (cov.size() != n) throw CombineERROR("covariant");
        covariants.push_back(cov);
        m++;
    }
    size_t dim() const { return m; }
    size_t size() const { return n; }
    friend class GWAS_row;
    void print() const;

};  // Gwas class for logic regression

class GWAS_row : public Row {
    Loci loci;
    Alleles alleles;
    size_t n;
    vector<double> data;
    vector<double> b;
    vector<double> y_est;
    const GWAS_logic &gwas;
    double estimate_y(size_t i);  // estimate y for the ith element
    void update_estimate();
    SqrMatrix H();
    double L();
    vector<double> Grad();

   public:
    GWAS_row(const GWAS_logic &_gwas) : gwas(_gwas), n(0) {}
    void read(string &line);
    void init() {
        b = vector<double>(gwas.dim(), 0);
        update_estimate();
    }  // m for dimension of beta
    size_t size() { return n; }
    void append_invalid_elts(size_t _n);
    void combine(const Row *other);
    void update_beta();
    bool fit(size_t max_iteration = 25, double sig = 0.000001);
    /* return true if converge, return false if explode*/
    vector<double> &beta() { return b; }
    double SE();
    double t_stat() { return b[0] / SE(); }
    Loci getloci() { return loci; }
    Alleles getallels() { return alleles; }
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

/* Buffer Management */

class Batch {
    string h;
    char crypt[ENCLAVE_READ_BUFFER_SIZE];
    Row_T type;
    deque<string> rows;
    deque<Loci> locus;
    bool r;  // ready if has been decrypted and isn't empty
    bool e;  // true if this batch  is the end of file.

   public:
    Batch(Row_T _type, string _host)
        : type(_type), h(_host), r(false), e(false) {}
    string host() { return h; }
    void decrypt();
    bool end() { return e; }
    bool ready() { return r; }
    Loci toploci() { return locus.front(); }
    // catch if emtpy before calling this!
    void pop();
    string &toprow() { return rows.front(); }
    friend class Buffer;
};

class Buffer {
    vector<Batch *> to_decrypt;
    vector<Batch *> working;
    map<string, int> host_map;
    vector<string> hosts;
    vector<size_t> host_col_num;
    Row_T type;

   public:
    Buffer(Row_T _type) : type(_type) {}
    void add_host(string _host, size_t _size) {
        hosts.push_back(_host);
        host_col_num.push_back(_size);
    }
    void init();  // init after adding all the hosts
    void load_batch();
    Row *get_nextrow(const GWAS_logic &gwas = GWAS_logic());
    // return nullptr if reaches end of all datasets
};

class OutputBuffer {
    char buffer[ENCLAVE_OUTPUT_BUFFER_SIZE];
    Row_T type;
    size_t size = 0;

   public:
    OutputBuffer(Row_T _type) : type(_type), size(0) {}
    bool extend(const string &);  // return false if buffer is full
    char *copy_to_host();
    void print() const { printf("%s", buffer); }
};

#endif