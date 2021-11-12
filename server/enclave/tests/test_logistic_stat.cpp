#include <cstring>
#include <fstream>
#include <iostream>

#include "logistic_regression.h"

#define ISFEMALEFILE \
    "../../archive/samples/1kg-logistic-regression/isFemale.tsv"
#define YFILE "../../archive/samples/1kg-logistic-regression/PurpleHair.tsv"

int main() {
    cout << scientific;
    cerr << scientific;
    try {
        uint8_t testarray[] = {
            0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 2, 0, 0,        0,
            0, 0, 0, 0, NA_uint8, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0,        0,
            1, 0, 0, 2, 0,        1, 0, 0, 0, 1, 0, 0, 0, 0, 0,        0,
            1, 1, 0, 0, 1,        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        0,
            0, 0, 0, 0, 0,        0, 0, 1, 1, 0, 1, 0, 0, 0, 0,        2,
            0, 0, 1, 1, NA_uint8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,        2,
            0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 1, 0, 0, 0, 1,        1,
            0, 0, 0, 0, 0,        1, 0, 0, 0, 0, 1, 0, 0, 0, 0,        1,
            0, 0, 0, 0, 0,        1, 0, 2, 1, 0, 0, 0, 1, 0, NA_uint8, 1,
            0, 1, 0, 0, 0,        0, 0, 0, 1, 0, 0, 0, 1, 0, 0,        0,
            1, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        0,
            0, 0, 0, 0, 0,        0, 0, 1, 0, 0, 1, 0, 1, 0, 0,        2,
            0, 0, 1, 0, 1,        0, 0, 0, 0, 0, 0, 1, 0, 0, 0,        1,
            0, 0, 0, 2, 1,        0, 0, 2, 2, 0, 0, 1, 0, 0, 1,        0,
            0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,        0,
            0, 0, 0, 0, 0,        0, 0, 0, 0, 0};
        string loci1 = "1:904165\t[\"G\",\"A\"]\t";
        char* line =
            new char[sizeof(testarray) / sizeof(char) + loci1.length()+1];
        strcpy(line, loci1.c_str());
        for (size_t i = loci1.length();
             i < sizeof(testarray) / sizeof(char) + loci1.length(); i++) {
            line[i] = (char)testarray[i - loci1.length()];
        }
        line[sizeof(testarray) / sizeof(char) + loci1.length()] = '\n';

        uint8_t testarray1[] = {
            0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0,
            0, 0, 0, NA_uint8, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
            0, 2, 0, 1,        0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0,
            0, 1, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1,        1, 0, 1, 0, 0, 0, 0, 2, 0, 0, 1, 1, NA_uint8,
            0, 0, 0, 0,        0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0};

        char* line1 =
            new char[sizeof(testarray1) / sizeof(char) + loci1.length()];
        strcpy(line1, loci1.c_str());
        for (size_t i = loci1.length();
             i < sizeof(testarray1) / sizeof(char) + loci1.length(); i++) {
            line1[i] = (char)testarray1[i - loci1.length()];
        }

        uint8_t testarray2[] = {
            0, 0, 0, 0, 0,        0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 1, 0,        0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 2, 1, 0,
            0, 0, 1, 0, NA_uint8, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
            0, 0, 0, 1, 0,        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,        0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 2, 0, 0, 1,
            0, 1, 0, 0, 0,        0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 2, 1, 0,
            0, 2, 2, 0, 0,        1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        char* line2 =
            new char[sizeof(testarray2) / sizeof(char) + loci1.length()];
        strcpy(line2, loci1.c_str());
        for (size_t i = loci1.length();
             i < sizeof(testarray2) / sizeof(char) + loci1.length(); i++) {
            line2[i] = (char)testarray2[i - loci1.length()];
        }

        ifstream if_y(YFILE);
        if (!if_y.is_open()) throw ReadtsvERROR("fail to open file");
        Log_var y;
        y.read(if_y);
        if_y.close();

        ifstream if_cov(ISFEMALEFILE);
        if (!if_cov.is_open()) throw ReadtsvERROR("fail to open file");
        Log_var isfemale;
        isfemale.read(if_cov);
        if_cov.close();

        Log_var intercept(y.size());

        Log_gwas gwas(y);  // 0.1 is step coefficient for each iteration
        gwas.add_covariant(intercept);
        gwas.add_covariant(isfemale);

        // Log_row row1(sizeof(testarray1) / sizeof(char));
        // row1.read(line1);
        // // row1.print();

        // Log_row row2(sizeof(testarray2)/ sizeof(char));
        // row2.read(line2);
        // row1.combine(&row2);
        // row1.print();

        Log_row row(sizeof(testarray) / sizeof(char));
        row.read(line);
        row.print();
        if (row.fit(&gwas))
            cout << "converged : true" << endl;
        else
            cout << "coverged : false" << endl;
        cout << "beta:";
        for (auto xx : row.beta()) cout << "\t" << xx;
        cout << endl;
        cout << "t_stat: " << row.t_stat() << endl;

        // if (row1.fit(&gwas))
        //     cout << "converged : true" << endl;
        // else
        //     cout << "coverged : false" << endl;
        // cout << "beta:";
        // for (auto xx : row1.beta()) cout << "\t" << xx;
        // cout << endl;
        // cout << "t_stat: " << row1.t_stat() << endl;

    } catch (ReadtsvERROR& error) {
        cerr << "ERROR: read tsv " << error.msg << endl;
    } catch (CombineERROR& error) {
        cerr << "ERROR: fail to combine " << error.msg << endl;
    } catch (ENC_ERROR& err) {
        cerr << "ERROR: ENC_ERROR " << err.msg << endl;
    }
}
