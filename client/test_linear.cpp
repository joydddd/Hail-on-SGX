#include "gwas.h"
#include <iostream>

using namespace std;

#define Y_FILE "../samples/1kg-linear-regression/Caffeine_2.tsv"
#define SNP_FILE "../samples/1kg-linear-regression/alleles_2.tsv"
#define COV_FILE {"1", "../samples/1kg-linear-regression/isfemale_2.tsv"}
#define XTX_OUTPUT_FILE "XTX.tsv"
#define XTY_OUTPUT_FILE "XTY.tsv"

#define BETA_FILE "withcovariant_beta.tsv"
#define SSE_OUTPUT_FILE "SSE.tsv"

int main() {
    try{
        vector<string> cov_files = COV_FILE;
        GWAS gwas(Y_FILE, cov_files);
        gwas.read_alleles(SNP_FILE);
        export_XTX_XTY(gwas, XTX_OUTPUT_FILE, XTY_OUTPUT_FILE);
        export_SSE(gwas, BETA_FILE, SSE_OUTPUT_FILE);
    } catch (ERROR_t &error) {
        cerr << "ERROR: " << error.msg << endl;
        exit(1);
    }
}