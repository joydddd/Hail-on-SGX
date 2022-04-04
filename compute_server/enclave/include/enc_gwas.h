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

// Virual class for row construction 
class Row {
    protected:
     /* meta data */
     Loci loci;
     Alleles alleles;
     size_t n;
     std::vector<uint8_t *> data;
     std::vector<size_t> length;

    public:
     /* return metadata */
     Loci getloci() { return loci; }
     Alleles getalleles() { return alleles; }
     size_t size() { return n; }


     /* setup */
     Row(size_t size);
     size_t read(const char line[], std::vector<std::string>& parts); // return the size of line consumed
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
    //parts.clear();
    // part.clear();
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

#endif