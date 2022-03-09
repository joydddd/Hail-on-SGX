#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <assert.h>

#include "gwas.h"

using namespace std;



int read_entry_int(string s){
    int xx;
    try {
        xx = stoi(s);
    } catch (invalid_argument &error) {
        if (s == "NA" || s == "NaN")
            xx = NA;
        else if (s == "false" || s == "False")
            xx = 0;
        else if (s == "true" || s == "True")
            xx = 1;
        else
            throw ReadtsvERROR("invalid entry: " + s);
    }
    return xx;
}

double read_entry_double(string s){
    double xx;
    try{
        xx = stod(s);
    } catch (invalid_argument &error){
        if (s == "NA" || s == "NaN")
            ReadtsvERROR("covariant/y entries must be valid");
        else if (s == "false" || s == "False")
            xx = 0;
        else if (s == "true" || s == "True")
            xx = 1;
        else 
            throw ReadtsvERROR("invalid entry: " + s);
    }
    return xx;
}

void split_allele( string &line, vector<string> &parts, string &loci, string &allele){
    string part;
    stringstream ss(line);
    if (!getline(ss, loci, '\t')) throw ReadtsvERROR("alleles file format");
    if (!getline(ss, allele, '\t')) throw ReadtsvERROR("alleles file format");
    while (getline(ss, part, '\t')){
        parts.push_back(part);
    }
}

void GWAS_var::mult(const vector<double> &x, vector<double> &ans){
    if (x.size() != data.size()) throw ERROR_t("vector length unmatch");
    ans.clear();
    ans.resize(x.size());
    for(size_t i=0; i<x.size(); i++){
        ans[i] = data[i] * x[i];
    }
}

void GWAS_var::prepare_XTX(vector<GWAS_var>::iterator me, vector<GWAS_var>::iterator end){
    if (prepared_XTX) return;
    while(me != end){
        XTX_xi.push_back(vector<double>());
        me->mult(data, XTX_xi.back());
        me++;
    }
    prepared_XTX = true;
}

void GWAS_var::prepare_XTY(){
    if (prepared_XTY) return;
    y->mult(data, XTY_x);
    prepared_XTY = true;
}

void GWAS_var::read_ref(ifstream &fs, string &key, const vector<string> &ref){
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
        data.push_back(read_entry_double(parts[1]));
    }
}

void GWAS_var::read(ifstream &fs, string &key, vector<string> &col_keys){
    string line;
    if (!getline(fs, line)) throw ReadtsvERROR("empty");
    vector<string> parts;
    if (split_tab(line, parts) != 2) throw ReadtsvERROR("title format " + line);
    key = parts[1];
    while(getline(fs, line)){
        if (split_tab(line, parts) != 2) throw ReadtsvERROR("format " + line);
        col_keys.push_back(parts[0]);
        data.push_back(read_entry_double(parts[1]));
    }
}

void GWAS_var::build_1(size_t length){
    data = vector<double>(length, 1);
}

double GWAS_var::dot_mult(const vector<int> &x)const{
    if (x.size() != data.size())
        throw mathERROR("vector dot mult unmatch length");
    double sum = 0;
    for (size_t i = 0; i < data.size(); i++) {
        if (x[i] != NA)
            sum += (double)x[i] * data[i];
    }
    return sum;
}

void GWAS_var::print_XTX(ostream &fs, const vector<int> &NA_ref, vector<GWAS_var>::iterator me, vector<GWAS_var>::iterator end){
    prepare_XTX(me, end);
    if (NA_ref.size() != data.size()) throw mathERROR("XTX: vector length unmatch");
    double sum = 0;
    for (auto & xtx:XTX_xi){
        for (size_t i=0; i<data.size(); i++){
            if (NA_ref[i] != NA) sum += xtx[i];
        }
        fs << "\t" << sum;
        sum = 0;
    }
}

void GWAS_var::print_XTY(ostream &fs, const vector<int> &NA_ref){
    prepare_XTY();
    if (NA_ref.size() != data.size()) throw mathERROR("XTY: vector length unmatch");
    double sum =0;
    for (size_t i=0; i<data.size(); i++){
        if (NA_ref[i] != NA) sum += XTY_x[i];
    }
    fs << "\t" << sum;
}

double GWAS_row::X2()const{
    double sum = 0;
    for(int xx:data){
        if (xx != NA) sum += xx*xx;
    }
    return sum;
}


GWAS_row::GWAS_row(string &line){
    vector<string> parts;
    string loci_str, alleles_str;
    split_allele(line, parts, loci_str, alleles_str);
    for (string &s : parts) {
        data.push_back(read_entry_int(s));
    }
    loci = Loci(loci_str);
    if (alleles.read(alleles_str)) inverse();
}

void GWAS_row::inverse(){
    ALLELE tmp = alleles.a1;
    alleles.a1 = alleles.a2;
    alleles.a2 = tmp;
    for (auto &x:data){
        if (x != NA) x = 2 - x;
    }
}

void GWAS_row::print_XTX(std::ostream &fs, vector<GWAS_var> &covariants) const {
    fs << loci << "\t" << alleles;
    fs << "\t" << X2();
    for (auto & cov: covariants){
        fs << "\t" << cov.dot_mult(data);
    }
    auto cov_it = covariants.begin();
    while(cov_it != covariants.end()){
        cov_it->print_XTX(fs, data, cov_it, covariants.end());
        cov_it++;
    }
    fs << endl;
}

void GWAS_row::print_XTY(std::ostream &fs, GWAS_var &y, vector<GWAS_var> &covariants) const{
    fs << loci << "\t" << alleles;
    fs << "\t" << y.dot_mult(data);
    for(auto &cov: covariants){
        cov.print_XTY(fs, data);
    }
    fs << endl;
}

void GWAS_row::print_SSE(ostream &fs, vector<double> &beta, vector<GWAS_var> &covariants, GWAS_var &y){
    fs << loci << "\t" << alleles;
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
    fs << "\t" << n << "\t" << SSE << endl;
}

void GWAS_row::print(){
    cout << loci << "\t" << alleles;
    for (auto xx:data) cout << "\t"<<xx;
    cout << endl;
}

Beta_row::Beta_row(vector<string> &parts, string &loci_str, string &allele_str):loci(loci_str){
    alleles.read(allele_str);
    beta.reserve(parts.size());
    for(auto& str:parts){
        beta.push_back(read_entry_double(str));
    }
     
}

GWAS::GWAS(string y_file, vector<string> &cov_files){
    read_y(y_file);
    for (auto &cov_file : cov_files) read_covariant(cov_file);
}

void GWAS::read_y(string filename) {
    ifstream tsv(filename);
    if (!tsv.is_open()) throw ReadtsvERROR("fail to open " + filename);
    y.read(tsv, y_key, col_keys);
    tsv.close();
}

void GWAS::read_covariant(string filename) {
    if (filename == "1") {
        covariant.push_back(GWAS_var());
        covariant.back().build_1(y.size());
    } else{
        ifstream tsv(filename);
        if (!tsv.is_open()) throw ReadtsvERROR("fail to open " + filename);
        covariant.push_back(GWAS_var());
        covariant_key.push_back(string());
        covariant.back().read_ref(tsv, covariant_key.back(), col_keys);
        tsv.close();
    }
    covariant.back().y = &y;
}

void GWAS::read_alleles(string filename){
    ifstream tsv;
    tsv.open(filename);
    if (!tsv.is_open()) throw ReadtsvERROR("fail to open " + filename);
    string line, loci, allele;
    if (!getline(tsv, line)) throw ReadtsvERROR("empty");
    vector<string> new_col_keys;
    split_allele(line, new_col_keys, loci, allele);
    if (new_col_keys != col_keys) throw ReadtsvERROR("unmatched col keys");
    if (loci != "locus" || allele != "alleles")
        throw ReadtsvERROR("alleles title format");
    while (getline(tsv, line)){
        try{data.push_back(line);
        } catch (ReadtsvERROR &error) {
            cerr << "read tsv ERROR in line " << line << " " << error.msg << endl << std::flush;
        }
    }
    tsv.close();
}

void GWAS::print_XTX_title(ostream &fs){
    fs << "locus\talleles";
    for (size_t i = 0; i < covariant.size() + 1; i++) {
        for (size_t j = i; j < covariant.size() + 1; j++){
            fs << "\t" << "XTX" << i + 1 << j + 1;
        }
    }
    fs << endl;
}
void GWAS::print_XTX(ostream &fs){
    for (auto &row:data) row.print_XTX(fs, covariant);
}

void GWAS::print_XTY(ostream &fs){
    for (auto &row:data) row.print_XTY(fs, y, covariant);
}

void GWAS::print_XTY_title(ostream &fs){
    fs << "locus\talleles\tXTY1";
    for (size_t i = 0; i < covariant.size(); i++) fs << "\t" << "XTY" << i + 2;
    fs << endl;
}

void GWAS::print_SSE(std::ostream &fs, std::vector<Beta_row> &betas){    
    auto beta_it = betas.begin();
    while(beta_it != betas.end() && sse_it != data.end()){
        if (beta_it->loci == sse_it->loci){
            try{
                if (!(beta_it->alleles == sse_it->alleles) )
                    throw ERROR_t("allele unmatch at loci "+ beta_it->loci.str());
                sse_it->print_SSE(fs, beta_it->beta, covariant, y);
            } catch(ERROR_t &error) {cerr << "Allele ERROR: " << error.msg << endl << std::flush;}
            sse_it++;
            beta_it++;
        }
        else if (beta_it->loci > sse_it->loci) sse_it++;
        else beta_it++;
    }
}

void GWAS::print(){
    cout << "Print total " << data.size() << " rows" << endl;;
    for(auto &row:data) row.print();
}

void export_XTX_XTY(GWAS &gwas, string XTX_filename, string XTY_filename){
    ofstream XTX_f(XTX_filename);
    if (!XTX_f.is_open()) throw exportERROR("fail to open "+XTX_filename);
    XTX_f << scientific;
    gwas.print_XTX_title(XTX_f);
    gwas.print_XTX(XTX_f);
    XTX_f.close();
    ofstream XTY_f(XTY_filename);
    XTY_f << scientific;
    if (!XTY_f.is_open()) throw exportERROR("fail to open " + XTY_filename);
    gwas.print_XTY_title(XTY_f);
    gwas.print_XTY(XTY_f);
    XTY_f.close();
}

void export_SSE(GWAS &gwas, string beta_filename, string SSE_filename){
    ifstream beta_f(beta_filename);
    if (!beta_f.is_open()) throw ReadtsvERROR("fail to open file "+beta_filename);
    ofstream SSE_f(SSE_filename);
    SSE_f << scientific;
    if (!SSE_f.is_open())
        throw exportERROR("fail to open file " + SSE_filename);
    string line;
    getline(beta_f, line);
    vector<Beta_row> beta;
    while(getline(beta_f, line)){
        vector<string> parts;
        string loci_str, allele_str;
        split_allele(line, parts, loci_str, allele_str);
        try{
        beta.push_back(Beta_row(parts, loci_str, allele_str));
        } catch(ReadtsvERROR &error) {
            cerr << "read tsv ERROR: in line " << line << " " << error.msg << endl << std::flush;
        }
    }
    gwas.SSE_init();
    gwas.print_SSE_title(SSE_f);
    gwas.print_SSE(SSE_f, beta);
}