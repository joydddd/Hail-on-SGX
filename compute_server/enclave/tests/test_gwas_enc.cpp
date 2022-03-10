#include "enc_gwas.h"
;
int main() { 
    try{
    uint8_t testarray[] = {9, 1, 2, 3, 4, 23, 1, 2, 3, '\0'};
    Row row(9);
    string loc = "1:904165\t[\"G\",\"A\"]\t";
    string line((char*)testarray);
    line = loc + line;
    row.read(line.c_str());
    row.print();

    uint8_t testarray2[] = {9, 5, 2, 2, 4, 2, '\0'};
    Row row2(6);
    string loc2 = "1:1563691\t[\"T\",\"G\"]\t";
    string line2((char *)testarray2);
    line2 = loc + line2;
    row2.read(line.c_str());
    row2.print();

    row.combine(&row2);
    row.print();

    row.append_invalid_elts(10);
    row.print();
    } catch (ERROR_t &err) {
        cerr << "ERROR: " << err.msg << endl;
    }
}