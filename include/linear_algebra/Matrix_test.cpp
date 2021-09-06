#include "Matrix.h"
#include <iostream>

int main(){
    vector<vector<double>> matrix{{1, 2, 3}, {0, 1, 5}, {5, 6, 0}};
    SqrMatrix m(matrix);
    cout << m;
    cout << m.DET();
    cout << m.COF().T();
    cout << m.INV();
    cout << "*" << endl;
    vector<double> vec{0, 1, 2};
    cout << (m*vec)[2];
}