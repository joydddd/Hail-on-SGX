#ifndef GWAS_ENCLAVE_H
#define GWAS_ENCLAVE_H
#include <limits>
#include <limits.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "gwas.h"
/* provide Alleles & Loci */

#define NA_uint8 0x3
#define NA_uint UINT_MAX
#define NA_double 3.0
#define uint8_OFFSET 0

template <typename T>
inline bool is_NA(T a) {
    if (std::isnan(a)) return true;
    if (std::is_same<T, uint8_t>::value) return (a == NA_uint8);
    if (std::is_same<T, unsigned int>::value) return (a == NA_uint);
    if (std::is_same<T, double>::value) return (a == NA_double);
    return true;
    // std::cout << a << " " << (a == NA_uint8) << " " << (a == NA_double) << std::endl;
    // return true;
}

// utilities
double read_entry_int(std::string &entry);
double max(std::vector<double>& vec);
bool read_entry_bool(std::string& entry);


// Virual class for row construction 
class Row {
    protected:
     /* meta data */
     Loci loci;
     Alleles alleles;
     size_t n;
    //  std::vector<uint8_t> data;
     uint8_t *data;
     std::vector<size_t> length;
     size_t genotype_sum;
     size_t genotype_count;
     double genotype_average;
     size_t it_count;

     std::string loci_str;
     std::string alleles_str;

     ImputePolicy impute_policy;

    public:
     /* return metadata */
     Loci getloci() { return loci; }
     Alleles getalleles() { return alleles; }
     size_t size() { return n; }
     virtual bool fit(size_t max_iteration = 25, double sig = 1e-6) { return false; }
     virtual double get_beta() { return -1; }
     virtual double get_t_stat() { return -1; }
     virtual double get_standard_error() { return -1; }
     size_t get_iterations() { return it_count; }


     /* setup */
     Row(size_t size, ImputePolicy _impute_policy);
     size_t read(const char line[]); // return the size of line consumed
     void combine(Row *other);
     void append_invalid_elts(size_t size);
     void reset();

#ifdef DEBUG
     void print();
#endif

     /* destructor */
     virtual ~Row() {
        //  for (uint8_t *array : data) delete[] array;
     }
};



inline size_t split_delim(const char* line, std::vector<std::string> &parts, char delim='\t', int delim_to_parse=-1) {
    std::string part;

    int num_delim = 0;
    int idx = 0;
    while (line[idx] != '\0') {
        if (line[idx] != delim) {
            part += line[idx];
        } 
        else {
            // Don't add empty strings?
            if (part.length()) {
                parts.push_back(part);
            }
            if (++num_delim == delim_to_parse) {
                return parts.size();
            }
            part.clear();
        }
        idx++;
    }
    if (part.length()) parts.push_back(part);

    return parts.size();
}


class Covar {
    friend class Log_row;
    friend class Lin_row;
    friend class GWAS;
    std::vector<double> data;
    size_t n;
    std::string name_str;

   public:
    Covar() : n(0), name_str("NA") { }
    Covar(const char* input, int res_size = 0) { read(input, res_size); }
    int read(const char* input, int res_size = 0);
    void reserve(int total_row_size);
    void init_1_covar(int total_row_size);
    size_t size() { return n; }
    const std::string& name() { return name_str; }
};


/* gwas setup. contains information for covariant and meta data */
class GWAS {
    std::string name;
    size_t m;  // dimension
    size_t n;  // sample size
    EncAnalysis regtype;

   public:
    std::vector<Covar*> covariants;
    Covar* y;
    GWAS(EncAnalysis _regtype) : n(0), m(0), regtype(_regtype) {}
    GWAS(Covar *_y, EncAnalysis _regtype) : n(_y->size()), m(1), regtype(_regtype) { add_y(_y); }

    void add_y(Covar *_y) {
        n = _y->size();
        m = 1;
        y = _y;
        switch (regtype){
            case EncAnalysis::logistic:
                name = _y->name() + "_logistic_gwas";
                break;
            case EncAnalysis::linear:
                name = _y->name() + "_linear_gwas";
                break;
        }

    }

    void add_covariant(Covar *cov) {
        if (cov->size() != n) throw CombineERROR("Covariant size did not match n");
        covariants.push_back(cov);
        m++;
    }
    size_t dim() const { return m; }
    size_t size() const { return n; }
#ifdef DEBUG
    void print() const;
#endif

};  // Gwas class for logic regression

#endif