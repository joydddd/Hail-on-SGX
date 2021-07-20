#include <iostream>
#include <fstream>
#include "gwas.h"

#define ISFEMALEFILE "../samples/1kg-logistic-regression/isFemale.tsv"
#define YFILE "../samples/1kg-logistic-regression/PurpleHair.tsv"

int main(){
    cout << scientific;
    cerr << scientific;
    try{
        string line1 = "1:904165\t[\"G\",\"A\"]\t0	0	0	0	0	0	0	0	0	0	0	0	2	0	0	0	0	0	0	0	NA	1	0	1	0	0	0	0	0	1	0	0	1	0	0	2	0	1	0	0	0	1	0	0	0	0	0	0	1	1	0	0	1	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	1	1	0	1	0	0	0	0	2	0	0	1	1	NA	0	0	0	0	0	0	0	0	0	1	2	0	0	0	0	0	0	0	0	0	0	1	0	0	0	1	1	0	0	0	0	0	1	0	0	0	0	1	0	0	0	0	1	0	0	0	0	0	1	0	2	1	0	0	0	1	0";
        string line2 =
            "1:904165	[\"G\",\"A\"]\tNA	1	0	1	0	0	0	0	0	"
            "0	1	0	0	0	1	0	0	0	1	0	0	0	0	0	0	"
            "0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	"
            "1	0	0	1	0	1	0	0	2	0	0	1	0	1	0	0	"
            "0	0	0	0	1	0	0	0	1	0	0	0	2	1	0	0	"
            "2	2	0	0	1	0	0	1	0	0	0	0	0	0	0	0	"
            "0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	"
            "0	0	0";

        ifstream if_y(YFILE);
        if (!if_y.is_open()) throw ReadtsvERROR("fail to open file");
        GWAS_var y(if_y);
        if_y.close();

        ifstream if_cov(ISFEMALEFILE);
        if (!if_cov.is_open())
            throw ReadtsvERROR("fail to open file");
        GWAS_var isfemale(if_cov);
        if_cov.close();

        GWAS_var intercept(y.size());

        GWAS_logic gwas(y, 0.1);// 0.1 is step coefficient for each iteration
        gwas.add_covariant(intercept);
        gwas.add_covariant(isfemale);

        GWAS_row row(gwas, line1);
        GWAS_row row2(gwas, line2);
        row.combine(row2);
        row.init();
        for (int i = 0; i < 3; i++){
            row.update_beta();
            cerr << "i:"<< i << " beta: " << row.beta()[0] << " t_stat: " << row.beta()[0]/row.SE() << endl;
        }
        for (auto xx : row.beta()) cout << "\t" <<xx;
        cout << endl;

    } catch (ReadtsvERROR& error) {
        cerr << "ERROR: read tsv " << error.msg << endl;
    } catch (CombineERROR& error) {
        cerr << "ERROR: fail to combine " << error.msg << endl;
    }
}
