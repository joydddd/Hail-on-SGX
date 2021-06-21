#include "gwas.h"
#include <iostream>

using namespace std;

int main() {
    try{
        string y_file;
        cout << "Y filename: "<< flush;
        cin >> y_file;
        cout << "covariant filename: " <<flush;
        string cov_file;
        vector<string> cov_files;
        cin >> cov_file;
        // cov_files.push_back(cov_file);
        GWAS gwas(y_file, cov_files);
        string filename;
        cout << "Enter alleles filename: " << flush;
        cin >> filename;
        gwas.read_alleles(filename);
        cout << "Export XTX to file: " << flush;
        cin >> filename;
        gwas.export_XTX(filename);
        cout << "Exprot XTY to file: " << flush;
        cin >> filename;
        gwas.export_XTY(filename);
    } catch (ReadtsvERROR &error) {
        cerr << "Read tsv ERROR: " << error.msg << endl;
        exit(1);
    }
}