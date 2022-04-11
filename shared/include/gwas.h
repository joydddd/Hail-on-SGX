#ifndef GWAS_H
#define GWAS_H

#include <iostream>
#include <sstream>
#include <string>
#include "buffer_size.h"
#include "gwas_error.h"
//#include "parser.h"


#define LOCI_X 23

enum Regression_T { Logistic, Linear };
enum ALLELE : char { A = 'A', T = 'T', C = 'C', G = 'G', NaN = 'N' };

class Loci {
   public:
    int chrom;
    int loc;
    Loci():chrom(0), loc(0){}
    Loci(const std::string &str);
    // Reuse dynamic memory!
    std::string chrom_str;
    std::string loc_str;

    friend std::ostream &operator<<(std::ostream &os, const Loci &loci) {
        if (loci.chrom == LOCI_X)
            os << "X";
        else
            os << loci.chrom;
        os << ":" << (loci.loc);
        return os;
    }
    friend bool operator<(const Loci &a, const Loci &b) {
        if (a.chrom < b.chrom) return true;
        if (a.chrom > b.chrom) return false;
        return a.loc < b.loc;
    }
    friend bool operator==(const Loci &a, const Loci &b) {
        return a.loc == b.loc && a.chrom == b.chrom;
    }
    friend bool operator!=(const Loci &a, const Loci &b) { return !(a == b); }
    friend bool operator>(const Loci &a, const Loci &b) {
        return !(a < b || a == b);
    }
    std::string str();
};

const Loci Loci_MAX("24:0");

class Alleles {
   public:
    ALLELE a1, a2;
    Alleles() : a1(NaN), a2(NaN) {}
    bool read(std::string str);  // return true if a1 <= a2
    void inverse();
    friend std::ostream &operator<<(std::ostream &os, const Alleles &alleles) {
        os << "[\"" << (char)alleles.a1 << "\",\"" << (char)alleles.a2 << "\"]";
        return os;
    }
    friend bool operator==(const Alleles &a, const Alleles &b) {
        return a.a1 == b.a1 && a.a2 == b.a2;
    }

    friend bool operator!=(const Alleles &a, const Alleles &b) {
        return !(a == b);
    }

    std::string str(const Loci &loci);
};

inline Loci::Loci(const std::string &str) {
    try {
        chrom_str.clear();
        loc_str.clear();
        bool chrom = true;
        for (char c : str) {
            if (c == ':') {
                chrom = false;
                continue;
            }
            if (chrom) {
                chrom_str.push_back(c);
            } else {
                loc_str.push_back(c);
            }
        }
        if (!chrom_str.length() || !loc_str.length()) {
            throw ReadtsvERROR("Bad input str: " + str);
        }
    } catch (std::invalid_argument &error) {
        throw ReadtsvERROR("Unknown Loci: " + str);
    }
}

inline void loci_to_str(const Loci &loci, std::string& loci_string) {
    loci_string.clear();

    if (loci.chrom == LOCI_X) {
            loci_string += "X";
    }
    else {
        loci_string += std::to_string(loci.chrom);
    }
    loci_string += ":" + std::to_string(loci.loc);
}

inline bool Alleles::read(std::string str) {
    if (str.length() != 9) throw ReadtsvERROR("Unknow alleles type " + str);
    char ma, mi;
    ma = str[2];
    switch (ma) {
        case 'A':
            a1 = ALLELE::A;
            break;
        case 'T':
            a1 = ALLELE::T;
            break;
        case 'C':
            a1 = ALLELE::C;
            break;
        case 'G':
            a1 = ALLELE::G;
            break;
        default:
            throw ReadtsvERROR("Unknow allele type " + str);
    }
    mi = str[6];
    switch (mi) {
        case 'A':
            a2 = ALLELE::A;
            break;
        case 'T':
            a2 = ALLELE::T;
            break;
        case 'C':
            a2 = ALLELE::C;
            break;
        case 'G':
            a2 = ALLELE::G;
            break;
        default:
            throw ReadtsvERROR("Unknow allele type " + str);
    }
    if (a1 < a2) {
        return true;
    }
    return false;
}

inline void Alleles::inverse() {
    ALLELE tmp = a1;
    a1 = a2;
    a2 = tmp;
}

inline void alleles_to_str(const Alleles &alleles, std::string& allele_string) {
    allele_string.clear();

    allele_string += "[\"";
    allele_string.push_back((char)alleles.a1);
    allele_string += "\",\"";
    allele_string.push_back((char)alleles.a2);
    allele_string += + "\"]";
}

#endif