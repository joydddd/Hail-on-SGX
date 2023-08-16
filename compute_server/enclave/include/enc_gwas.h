#ifndef GWAS_ENCLAVE_H
#define GWAS_ENCLAVE_H
#include <limits>
#include <limits.h>
#include <stdio.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "Matrix.h"
#include "gwas.h"
/* provide Alleles & Loci */

#define NA_byte 0xFF
#define NA_uint8 0x3
#define NA_uint UINT_MAX
#define NA_double 3.0
#define uint8_OFFSET 0

#define DOUBLE_CACHE_BLOCK (int)(64 / sizeof(double))

inline int get_padded_buffer_len(int n) {
    return (((n % DOUBLE_CACHE_BLOCK) != 0) + (n / DOUBLE_CACHE_BLOCK)) * DOUBLE_CACHE_BLOCK * 4;
}

// template <typename T>
// inline bool is_NA(T a) {
//     if (std::isnan(a)) return true;
//     if (std::is_same<T, uint8_t>::value) return (a == NA_uint8);
//     if (std::is_same<T, unsigned int>::value) return (a == NA_uint);
//     if (std::is_same<T, double>::value) return (a == NA_double);
//     return true;
// }

inline bool is_NA_uint8(uint8_t val) {
    return val == NA_uint8;
}

inline uint8_t is_not_NA_oblivious(uint8_t val) {
    return !(((val & 2) >> 1) & (val & 1));
}

// utilities
double read_entry_int(std::string &entry);
double bd_max(const double *vec, int len);
double bd_max(const std::vector<double>& vec);
bool read_entry_bool(std::string& entry);


// Virual class for row construction 
class Row {
    protected:
     /* meta data */
     Loci loci;
     Alleles alleles;
     int n;
     int num_dimensions;
     int read_row_len;
    //  std::vector<uint8_t> data;
     uint8_t *data;
     std::vector<int> client_lengths;
     int genotype_sum;
     int genotype_count;
     double genotype_average;
     int it_count;

     std::string loci_str;
     std::string alleles_str;

     ImputePolicy impute_policy;

     bool impute_average;

    public:
     /* return metadata */
     Loci getloci() { return loci; }
     Alleles getalleles() { return alleles; }
     int size() { return n; }
     virtual bool fit(int thread_id = -1, int max_iteration = 15, double sig = 1e-6) { std::cout << "WARNING: GENERIC FIT!?!" << std::endl; return false; }
     virtual double get_beta(int thread_id) { return -1; }
     virtual double get_t_stat(int thread_id) { return -1; }
     virtual double get_standard_error(int thread_id) { return -1; }
     virtual void get_outputs(int thread_id, std::string& output_string) {};
     int get_iterations() { return it_count; }



     /* setup */
     Row(int size, const std::vector<int>& sizes, int _num_dimensions, ImputePolicy _impute_policy);
     int read(const char line[]); // return the size of line consumed
     void combine(Row *other);
     void append_invalid_elts(int size);
     void reset();
    

#ifdef DEBUG
     void print();
#endif

     /* destructor */
     virtual ~Row() {
        //  for (uint8_t *array : data) delete[] array;
     } 
};



inline int split_delim(const char* line, std::vector<std::string> &parts, char delim='\t', int delim_to_parse=-1) {
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
    friend class Oblivious_lin_row;
    friend class Oblivious_log_row;
    friend class GWAS;
    std::vector< std::vector<double> > data;
    int n;
    int m;
    int covar_idx;
    std::string name_str;

   public:
    Covar() : n(0), name_str("NA") { }
    Covar(const char* input, int res_size = 0) { read(input, res_size); }
    Covar(int _n, int _m) : n(_n), m(0), covar_idx(0) {data.resize(_n, std::vector<double>(_m));}
    int read(const char* input, int res_size = 0);
    void reserve(int total_row_size);
    void init_1_covar(int total_row_size);
    int size() { return n; }
    const std::string& name() { return name_str; }
    void after_covar() {
        m++;
        covar_idx = 0;
    }
};


/* gwas setup. contains information for covariant and meta data */
class GWAS {
    int m;  // dimension
    int n;  // sample size
    EncAnalysis regtype;

   public:
    Covar phenotype_and_covars;
    GWAS(EncAnalysis _regtype) : n(0), m(0), regtype(_regtype) {}
    GWAS(EncAnalysis _regtype, int _n, int _m) : n(_n), m(_m), regtype(_regtype), phenotype_and_covars(_n, _m) {}

    // void add_y(Covar *_y) {
    //     n = _y->size();
    //     m = 1;
    //     y = _y;
    // }

    // void add_covariant(Covar *cov) {
    //     if (cov->size() != n) throw CombineERROR("Covariant size did not match n");
    //     covariants.push_back(cov);
    //     m++;
    //}
    int dim() const { return m; }
    int size() const { return n; }
#ifdef DEBUG
    void print() const;
#endif

};  // Gwas class for logic regression

extern double *beta_g;
extern GWAS *gwas;

#endif