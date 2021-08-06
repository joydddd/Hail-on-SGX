#include "gwas.h"

int main(){
    cout << scientific;
    cerr << scientific;
    try {
        string line1 = "1:904165\t[\"G\",\"A\"]\t31\t23\t13\t98\t55\t55";
        string line2 = "1:904165\t[\"G\",\"A\"]\t44\t34\t20\t149\t77\t77";
        XTX_row xtx;
        xtx.read(line1);
        XTX_row xtx2;
        xtx2.read(line2);
        xtx.combine(&xtx2);

        string xty_line1 = "1:904165\t[\"G\",\"A\"]\t116\t457\t243";
        string xty_line2 = "1:904165\t[\"G\",\"A\"]\t129\t651\t334";
        XTY_row xty, xty2;
        xty.read(xty_line1);
        xty2.read(xty_line2);
        xty.combine(&xty2);

        vector<double> beta;
        xtx.beta(beta, xty);

        string SSE_line1 = "1:904165\t[\"G\",\"A\"]\t98\t195.692";
        string SSE_line2 = "1:904165\t[\"G\",\"A\"]\t149\t414.643";
        SSE_row sse, sse2;
        sse.read(SSE_line1);
        sse2.read(SSE_line2);
        sse.combine(&sse2);

        for (auto xx : beta) {
            cout << xx << "\t";
        }
        cout << endl;
        cout << "P-value " << sse.p(xtx.INV(), beta) << endl;
    } catch (ReadtsvERROR& error) {
        cerr << "ERROR: read tsv " << error.msg << endl;
    } catch (CombineERROR& error) {
        cerr << "ERROR: fail to combine " << error.msg << endl;
    }
}