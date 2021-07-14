#include "gwas.h"

#include <boost/math/distributions/students_t.hpp>
#include <cmath>
using namespace std;
using namespace boost::math;
double CDF_students_t(double t, int df) { 
    students_t t_dist((double)df);
    return cdf(t_dist, t);
}

size_t split_tab(string &line, vector<string> &parts) {
    parts.clear();
    string part;
    stringstream ss(line);
    while (getline(ss, part, '\t')) {
        parts.push_back(part);
    }
    return parts.size();
}

XTX_row::XTX_row(string &line) {
    vector<string> parts;
    size_t xx = split_tab(line, parts);
    if (xx < 2) throw ReadtsvERROR("Line too short: " + line);
    loci = Loci(parts[0]);
    alleles.read(parts[1]);
    m = (size_t)sqrt((xx - 2) * 2);
    if (m * (m + 1) != (xx - 2) * 2)
        throw ReadtsvERROR("invalid XTX entry number: " + line);
    vector<vector<double>> XTX_vec;
    XTX_vec.resize(m);
    for (auto &l : XTX_vec) {
        l.resize(m);
    }
    size_t k = 2;
    for (size_t i = 0; i < m; i++) {
        XTX_vec[i].resize(m);
        for (size_t j = i; j < m; j++) {
            try {
                XTX_vec[i][j] = stod(parts[k]);
            } catch (invalid_argument &error) {
                throw ReadtsvERROR("Invalid XTX entry: " + parts[k]);
            }
            k++;
        }
    }
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < i; j++) {
            XTX_vec[i][j] = XTX_vec[j][i];
        }
    }
    XTX = SqrMatrix(XTX_vec);
}

void XTX_row::combine(const XTX_row &other) {
    if (other.loci != this->loci) throw CombineERROR("locus mismatch");
    if (other.alleles != this->alleles) throw CombineERROR("alleles mismatch");
    if (other.size() != this->size()) throw CombineERROR("size mismatch");
    XTX = this->XTX + other.XTX;
}

void XTX_row::beta(vector<double> &beta, XTY_row &xty) {
    if (m != xty.size()) throw CombineERROR("size mismatch");
    if (alleles != xty.alleles) throw CombineERROR("alleles mismatch");
    if (loci != xty.loci) throw CombineERROR("loci mismatch");
    beta = INV()* xty.XTY;
}

SqrMatrix& XTX_row::INV(){
    if (XTX_1.is_empty()) XTX_1 = XTX.INV();
    return XTX_1;
}

XTY_row::XTY_row(string str) {
    vector<string> parts;
    split_tab(str, parts);
    if (parts.size() < 2) throw ReadtsvERROR("Line too short: " + str);
    m = parts.size() - 2;
    loci = Loci(parts[0]);
    alleles.read(parts[1]);
    XTY.resize(m);
    for (size_t i = 0; i < m; i++) {
        try {
            XTY[i] = stod(parts[i + 2]);
        } catch (invalid_argument &error) {
            throw ReadtsvERROR("Invalid XTY entry: " + parts[i + 2]);
        }
    }
}

void XTY_row::combine(const XTY_row &other) {
    if (other.loci != loci) throw CombineERROR("locus mismatch");
    if (other.alleles != alleles) throw CombineERROR("alleles mismatch");
    if (other.m != m) throw CombineERROR("XTY size mismatch");
    for (size_t i = 0; i < m; i++) {
        XTY[i] += other.XTY[i];
    }
}

SSE_row::SSE_row(string &line) {
    vector<string> parts;
    split_tab(line, parts);
    if (parts.size() != 4) throw ReadtsvERROR("Line "+line);
    loci = Loci(parts[0]);
    alleles.read(parts[1]);
    try {
        n = stoi(parts[2]);
        SSE = stod(parts[3]);
    } catch (invalid_argument &error) {
        throw ReadtsvERROR("invalid entry " + parts[2] + " " + parts[3]);
    }
}

void SSE_row::combine(SSE_row &other) {
    if (other.loci != loci) throw CombineERROR("locus mismatch");
    if (other.alleles != alleles) throw CombineERROR("alleles mismatch");
    SSE += other.SSE;
    n += other.n;
}

double SSE_row::p(SqrMatrix &XTX_1, vector<double> &beta) {
    size_t m = beta.size();
    SqrMatrix var = XTX_1 * (SSE / (double)(n - m - 1));
    double SE = sqrt(var[0][0]);
    double T = beta[0] / SE;
    return CDF_students_t(T, (int)(n - m - 1))*2;
}


