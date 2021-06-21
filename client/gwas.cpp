#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>

#include "gwas.h"

using namespace std;

double read_entry(string s){
    double xx;
    try {
        xx = stod(s);
    } catch (invalid_argument &error) {
        if (s == "NA")
            xx = NA;
        else if (s == "false" || s == "False")
            xx = 0;
        else if (s == "true" || s == "True")
            xx = 1;
        else
            throw ReadtsvERROR("unknow entry: " + s);
    }
    return xx;
}

double split_tab(string &line, vector<string> &parts){
    parts.clear();
    string part;
    stringstream ss(line);
    while(getline(ss, part, '\t')){
        parts.push_back(part);
    }
    return (int)parts.size();
}

void split_allele( string &line, vector<string> &parts, string &locu, string &allele){
    string part;
    stringstream ss(line);
    if (!getline(ss, locu, '\t')) throw ReadtsvERROR("alleles file format");
    if (!getline(ss, allele, '\t')) throw ReadtsvERROR("alleles file format");
    while (getline(ss, part, '\t')){
        parts.push_back(part);
    }
}

double GWAS_row::dot_mult(const GWAS_row &x, const vector<double> &valid) const {
    if (x.size() != data.size())
        throw mathERROR("vector dot mult unmatch length");
    double sum = 0;
    for (size_t i = 0; i < data.size(); i++) {
        if (valid[i] != NA)
            sum += x.data[i] * data[i];
    }
    return sum;
}

void GWAS_row::XTX(vector<GWAS_row>::iterator co_begin,
                   vector<GWAS_row>::iterator co_end, vector<double>::iterator ans,
                   const GWAS_row &NA_ref) const {
    *ans = dot_mult(*this, NA_ref.data);
    ans++;
    if (co_begin == co_end) return;
    for (auto it = co_begin; it != co_end; it++) {
        *ans = dot_mult(*it, NA_ref.data);
        ans++;
    }
    co_begin->XTX(co_begin + 1, co_end, ans, NA_ref);
}

void GWAS_row::XTY(GWAS_row &y, vector<GWAS_row> &covariants, vector<double> &ans) const {
    ans.clear();
    ans.resize(covariants.size()+1);
    ans[0] = dot_mult(y, data);
    for (size_t i=0; i<covariants.size(); i++){
        ans[i + 1] = covariants[i].dot_mult(y, data);
    }
}

pair<double, int> GWAS_row::SSE(vector<double> &beta, vector<GWAS_row> &covariants, GWAS_row&y){
    int n = 0;
    double SSE = 0;
    for (size_t k=0; k<data.size(); k++){
        if (data[k] != NA){
            double yy = data[k]*beta[0];
            for (size_t i=0; i<covariants.size(); i++){
                yy += covariants[i].data[k] * beta[i + 1];
            }
            SSE += (yy - y.data[k]) * (yy - y.data[k]);
            n++;
        }
    }
    return make_pair(SSE, n);
}

void GWAS_row::read_row(string &line, string &locu, string &allele) {
    vector<string> parts;
    split_allele(line, parts, locu, allele);
    data.reserve(parts.size() - 1);
    for (string &s : parts) {
        data.push_back(read_entry(s));
    }
}

void GWAS_row::read_col_ref(ifstream &fs, string &key, const vector<string> &ref) {
    string line;
    if (!getline(fs, line)) throw ReadtsvERROR("empty");
    vector<string> parts;
    if (split_tab(line, parts) != 2) throw ReadtsvERROR("title format" + line);
    key = parts[1];
    data.reserve(ref.size());
    for (string ref_key : ref) {
        if (!getline(fs, line)) throw ReadtsvERROR("unmatched col keys");
        split_tab(line, parts);
        if (parts[0] != ref_key) throw ReadtsvERROR("unmatched col keys");
        data.push_back(read_entry(parts[1]));
    }
}

void GWAS_row::read_col(ifstream &fs, string &key, vector<string> &col_keys){
    string line;
    if (!getline(fs, line)) throw ReadtsvERROR("empty");
    vector<string> parts;
    if (split_tab(line, parts) != 2) throw ReadtsvERROR("title format " + line);
    key = parts[1];
    while(getline(fs, line)){
        if (split_tab(line, parts) != 2) throw ReadtsvERROR("format " + line);
        col_keys.push_back(parts[0]);
        data.push_back(read_entry(parts[1]));
    }
}

GWAS::GWAS(string &y_file, vector<string> &cov_files){
    read_y(y_file);
    for (auto &cov_file : cov_files) read_covariant(cov_file);
}

void GWAS::read_y(string &filename) {
    ifstream tsv(filename);
    if (!tsv.is_open()) throw ReadtsvERROR("fail to open " + filename);
    y.read_col(tsv, y_key, col_keys);
    tsv.close();
}

void GWAS::read_covariant(string &filename) {
    ifstream tsv(filename);
    if (!tsv.is_open()) throw ReadtsvERROR("fail to open " + filename);
    covariant.push_back(GWAS_row());
    covariant_key.push_back(string());
    covariant.back().read_col_ref(tsv, covariant_key.back(), col_keys);
    tsv.close();
}

void GWAS::read_alleles(string &filename){
    data.clear();
    ifstream tsv;
    tsv.open(filename);
    if (!tsv.is_open()) throw ReadtsvERROR("fail to open " + filename);
    string line, locu, allele;
    if (!getline(tsv, line)) throw ReadtsvERROR("empty");
    vector<string> new_col_keys;
    split_allele(line, new_col_keys, locu, allele);
    if (new_col_keys != col_keys) throw ReadtsvERROR("unmatched col keys");
    if (locu != "locus" || allele != "alleles")
        throw ReadtsvERROR("alleles title format");
    while (getline(tsv, line)) {
        data.push_back(GWAS_row());
        locus.push_back(string());
        alleles.push_back(string());
        data.back().read_row(line, locus.back(), alleles.back());
    }
    tsv.close();
}

void GWAS::export_XTX(string &filename){
    ofstream fs(filename);
    if (!fs.is_open()) throw exportERROR("fail to open " + filename);
    fs << "locus\talleles\t";
    for (size_t i = 0; i < covariant.size() + 1; i++) {
        for (size_t j = i; j < covariant.size() + 1; j++){
            fs << "XTX" << i + 1 << j + 1 << "\t";
        }
    }
    fs << endl;
        vector<double> XTX;
    XTX.resize((covariant.size() + 2) * (covariant.size() +1)/2);
    for (size_t k=0; k<data.size(); k++){
        fs << locus[k] << "\t" << alleles[k] << "\t";
        data[k].XTX(covariant.begin(), covariant.end(), XTX.begin(), data[k]);
        for (auto &xx : XTX) fs << xx << "\t";
        fs << endl;
    }
    fs.close();
}

void GWAS::export_XTY(string &filename){
    ofstream fs(filename);
    if (!fs.is_open()) throw exportERROR("fail to open " + filename);
    fs << "locus\talleles\tXTY1\t";
    for (size_t i = 0; i < covariant.size(); i++) fs << "XTY" << i + 2 << "\t";
    fs << endl;
    for (size_t k=0; k<data.size(); k++){
        vector<double> XTY;
        data[k].XTY(y, covariant, XTY);
        fs << locus[k] << "\t" << alleles[k] << "\t";
        for (auto &xx : XTY) fs << xx << "\t";
        fs << endl;
    }
    fs.close();
}

void GWAS::export_SSE(map<string, vector<double>> &beta, string &filename){
    ofstream fs(filename);
    if (!fs.is_open()) throw exportERROR("fail to open " + filename);
    fs << "locus\tSSE\tn\t" << endl;
    for(size_t k=0; k<data.size(); k++){
        pair<double, int> sse;
        sse = data[k].SSE(beta[locus[k]], covariant, y);
        fs << locus[k] << "\t" << sse.first << "\t" << sse.second << endl;
    }
}

void GWAS::print(){
    cout << "col_key\t";
    for (string &s : col_keys) {
        cout << s << "\t";
    }
    cout << endl; 
    cout << y_key << "\t";
    for(double yy:y.data){
        cout << yy << "\t";
    }
    cout << endl;
    for(int i=0; i<(int)covariant_key.size(); i++){
        cout << covariant_key[i]<<"\t";
        for (double xx : covariant[i].data) cout << xx << "\t";
        cout << endl;
    }
    for (int i = 0; i < (int)locus.size(); i++){
        cout << locus[i] << alleles[i]<<"\t";
        for(double xx:data[i].data){
            cout << xx << "\t";
        }
        cout << endl;
    }
}
