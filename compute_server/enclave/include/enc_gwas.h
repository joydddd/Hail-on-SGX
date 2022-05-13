#ifndef GWAS_ENCLAVE_H
#define GWAS_ENCLAVE_H
#include <limits.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "gwas.h"
/* provide Alleles & Loci */

#define NA_uint8 0x3
#define NA_uint UINT_MAX
#define uint8_OFFSET 0

template <typename T>
inline bool is_NA(T a) {
    if (std::isnan(a)) return true;
    if (std::is_same<T, uint8_t>::value) return (a == NA_uint8);
    if (std::is_same<T, unsigned int>::value) return (a == NA_uint);
    return true;
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
     std::vector<uint8_t *> data;
     std::vector<size_t> length;

     std::string loci_str;
     std::string alleles_str;

    public:
     /* return metadata */
     Loci getloci() { return loci; }
     Alleles getalleles() { return alleles; }
     size_t size() { return n; }


     /* setup */
     Row(size_t size);
     size_t read(const char line[]); // return the size of line consumed
     void combine(Row *other);
     void append_invalid_elts(size_t size);
     void reset();

#ifdef DEBUG
     void print();
#endif

     /* destructor */
     virtual ~Row() {
         for (uint8_t *array : data) delete[] array;
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
            parts.push_back(part);
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
    Covar() : n(0), name_str("NA") {}
    Covar(std::istream &is) { read(is); }
    void read(std::istream &is);
    Covar(size_t size, int x = 1) : data(size, x), n(size), name_str("1") {}
    size_t size() { return n; }
    void combine(Covar &other);
    const std::string& name() { return name_str; }
    Covar &operator=(Covar &rhs) {
        if (this == &rhs) return *this;
        data = rhs.data;
        n = rhs.n;
        name_str = rhs.name_str;
        return *this;
    }
};

enum RegType_t{ LogReg_type, LinReg_type };


/* gwas setup. contains information for covariant and meta data */
class GWAS {
    std::string name;
    size_t m;  // dimention
    size_t n;  // sample size
    RegType_t regtype;

   public:
    std::vector<Covar> covariants;
    Covar y;
    GWAS() : n(0), m(0), regtype(LogReg_type) {}
    GWAS(Covar _y, RegType_t _regtype) : n(_y.size()), m(1), regtype(_regtype) { add_y(_y); }

    void add_y(Covar &_y) {
        n = _y.size();
        m = 1;
        y = _y;
        switch (regtype){
            case LogReg_type:
                name = _y.name() + "_logic_gwas";
                break;
            case LinReg_type:
                name = _y.name() + "_linear_gwas";
                break;
        }

    }

    void add_covariant(Covar &cov) {
        if (cov.size() != n) throw CombineERROR("covariant");
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