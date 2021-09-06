#include <iostream>
#include <fstream>
#include "enc_gwas.h"

#define ISFEMALEFILE "../../samples/1kg-logistic-regression/isFemale.tsv"
#define YFILE "../../samples/1kg-logistic-regression/PurpleHair.tsv"

int main(){
    // cout << scientific;
    // cerr << scientific;
    // try{
    //     string line1 = "1:904165\t[\"G\",\"A\"]\t0	0	0	0	0	0	0	0	0	0	0	0	2	0	0	0	0	0	0	0	NA	1	0	1	0	0	0	0	0	1	0	0	1	0	0	2	0	1	0	0	0	1	0	0	0	0	0	0	1	1	0	0	1	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	1	1	0	1	0	0	0	0	2	0	0	1	1	NA	0	0	0	0	0	0	0	0	0	1	2	0	0	0	0	0	0	0	0	0	0	1	0	0	0	1	1	0	0	0	0	0	1	0	0	0	0	1	0	0	0	0	1	0	0	0	0	0	1	0	2	1	0	0	0	1	0";
    //     string line2 =
    //         "1:904165	[\"G\",\"A\"]\tNA	1	0	1	0	0	0	0	0	"
    //         "0	1	0	0	0	1	0	0	0	1	0	0	0	0	0	0	"
    //         "0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	"
    //         "1	0	0	1	0	1	0	0	2	0	0	1	0	1	0	0	"
    //         "0	0	0	0	1	0	0	0	1	0	0	0	2	1	0	0	"
    //         "2	2	0	0	1	0	0	1	0	0	0	0	0	0	0	0	"
    //         "0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	"
    //         "0	0	0";

    //     ifstream if_y(YFILE);
    //     if (!if_y.is_open()) throw ReadtsvERROR("fail to open file");
    //     GWAS_var y(if_y);
    //     if_y.close();

    //     ifstream if_cov(ISFEMALEFILE);
    //     if (!if_cov.is_open())
    //         throw ReadtsvERROR("fail to open file");
    //     GWAS_var isfemale(if_cov);
    //     if_cov.close();

    //     GWAS_var intercept(y.size());

    //     GWAS_logic gwas(y);// 0.1 is step coefficient for each iteration
    //     gwas.add_covariant(intercept);
    //     gwas.add_covariant(isfemale);

    //     GWAS_row row(gwas);
    //     row.read(line1);
    //     GWAS_row row2(gwas);
    //     row2.read(line2);
    //     row.combine(&row2);
    //     row.init();
    //     if (row.fit()) cout << "converged : true" << endl;
    //     cout << "beta:";
    //     for (auto xx : row.beta()) cout << "\t" << xx;
    //     cout << endl;
    //     cout << "t_stat: " << row.t_stat() << endl;

    // } catch (ReadtsvERROR& error) {
    //     cerr << "ERROR: read tsv " << error.msg << endl;
    // } catch (CombineERROR& error) {
    //     cerr << "ERROR: fail to combine " << error.msg << endl;
    // }
}
