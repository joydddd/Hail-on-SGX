#ifndef GWAS_H
#define GWAS_H

#include <iostream>
#include <sstream>
#include <string>
#include "buffer_size.h"

#define EndSperator "<EOF>" // mark end of dataset

#include "gwas_error.h"

using namespace std;

enum Regression_T { Logistic, Linear };
enum ALLELE : char { A = 'A', T = 'T', C = 'C', G = 'G', NaN = 'N' };

class Loci {
   public:
    static const int X = 23;
    int chrom;
    int loc;
    Loci():chrom(0), loc(0){}
    Loci(const string &str);
    friend ostream &operator<<(ostream &os, const Loci &loci) {
        if (loci.chrom == X)
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
    string str();
};

const Loci Loci_MAX("24:0");

class Alleles {
   public:
    ALLELE a1, a2;
    Alleles() : a1(NaN), a2(NaN) {}
    bool read(string str);  // return true if a1 <= a2
    void inverse();
    friend ostream &operator<<(ostream &os, const Alleles &alleles) {
        os << "[\"" << (char)alleles.a1 << "\",\"" << (char)alleles.a2 << "\"]";
        return os;
    }
    friend bool operator==(const Alleles &a, const Alleles &b) {
        return a.a1 == b.a1 && a.a2 == b.a2;
    }

    friend bool operator!=(const Alleles &a, const Alleles &b) {
        return !(a == b);
    }
};

inline Loci::Loci(const string &str) {
    stringstream ss(str);
    string chrom_str, loc_str;
    getline(ss, chrom_str, ':');
    getline(ss, loc_str, ':');
    try {
        if (chrom_str == "X")
            chrom = X;
        else
            chrom = stoi(chrom_str);
        loc = stoi(loc_str);
    } catch (invalid_argument &error) {
        throw ReadtsvERROR("Unknown Loci: " + str);
    }
}

inline string Loci::str() {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

inline bool Alleles::read(string str) {
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

#endif