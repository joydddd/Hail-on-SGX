#ifndef MATRIX_H
#define MATRIX_H

using namespace std;
#include <vector>
#include <string>


#ifdef DEBUG
#include <sstream>
#include <iostream>
#endif

class MathError{
    public:
    string msg;
    MathError(string _msg):msg(_msg){}
};

class SqrMatrix{
    private:
    vector<vector<double>> m;
    size_t n;
    public: 
    SqrMatrix():n(0){}
    SqrMatrix(size_t s, int opt):m(s, vector<double>(s, 0)), n(s) {
        if (opt) {
            det = new SqrMatrix(s, 0);
        }
        if (opt == 2) {
            sub = new SqrMatrix(s - 1, 1);
            cof = new SqrMatrix(s, 0);
            t = new SqrMatrix(s, 0);
        }
        
    }
    ~SqrMatrix() {
        // if (det) delete det;
        // if (sub) delete sub;
        // if (cof) delete cof;
        // if (t) delete t;
    }
    SqrMatrix(vector<vector<double>> &vec):m(vec), n(vec.size()){
        if (n > 0 && m[0].size() != n)
            throw MathError("SqrMatrix: not a square matrix");
    }
    SqrMatrix* sub;
    SqrMatrix* cof;
    SqrMatrix* t;
    SqrMatrix* det;
    SqrMatrix(const SqrMatrix&) = default;
    SqrMatrix(SqrMatrix&&) = default;
    SqrMatrix& operator=(const SqrMatrix&) = default;
    SqrMatrix& operator=(SqrMatrix&) = default;
    vector<double>& operator[](size_t index){return m[index];}
    double at(size_t row, size_t col) const {return m[row][col];}
    size_t size() const {return n;}
    bool is_empty() { return n == 0; }
    // friend SqrMatrix operator+(const SqrMatrix& me, const SqrMatrix &other) {
    //     if (me.n != other.n) throw MathError("SqrMatrix size mistmatch");
    //     SqrMatrix sum(me.n);
    //     for (size_t i=0; i<me.n; i++){
    //         for (size_t j=0; j<me.n; j++){
    //             sum.m[i][j] = me.m[i][j] + other.m[i][j];
    //         }
    //     }
    //     return {sum};
    // }
    // friend SqrMatrix operator*(const SqrMatrix& me, double mult){
    //     SqrMatrix ans(me.n);
    //     for (size_t i=0; i<me.n; i++){
    //         for (size_t j=0; j<me.n; j++){
    //             ans[i][j] = me.m[i][j] * mult;
    //         }
    //     }
    //     return ans;
    // }
    // friend vector<double> operator*(const SqrMatrix& me, const vector<double>& mult){
    //     if (mult.size() != me.n) throw MathError("Matrix Vector dim mismatch");
    //     vector<double> ans(me.n, 0);
    //     for (size_t i=0; i<me.n; i++){
    //         for (size_t j=0; j<me.n; j++){
    //             ans[i] += me.m[i][j] * mult[j];
    //         }
    //     }
    //     return ans;
    // }

    void multiply_matrix(double mult){
        for (size_t i = 0; i < n; i++){
            for (size_t j = 0; j < n; j++){
                m[i][j] *= mult;
            }
        }
    }
    void calculate_beta_delta(const vector<double>& mult, vector<double>& beta_delta) const {
        if (mult.size() != n) throw MathError("Matrix Vector dim mismatch");
        for (size_t i = 0; i < n; i++){
            beta_delta[i] = 0;
            for (size_t j = 0; j < n; j++){
                beta_delta[i] += m[i][j] * mult[j];
            }
        }
    }

    #ifdef DEBUG
    friend ostream& operator<<(ostream& os, const SqrMatrix& matrix){
        for (auto &row:matrix.m){
            for (auto xx: row){
                os << xx << "\t";
            }
            os << endl;
        }
        return os;
    }
    #endif
    void T() const{
        for (size_t i = 0; i < n; i++){
            for (size_t j = 0; j < n; j++){
                (*t)[i][j] = (*cof)[j][i];
            }
        }
    }

    // vector<double> DIAG()const{
    //     vector<double> ans;
    //     ans.resize(n);
    //     for(size_t i=0; i<n; i++){
    //         ans[i] = m[i][i];
    //     }
    //     return ans;
    // }
    
    void COF() const {
        if (n == 1) {
            (*cof)[0][0] = 1;
            return;
        }
        for(int i = 0; i < n; ++i) {
            for(int j = 0; j < n; ++j) {
                (*cof)[i][j] = 0;
                if (i < n - 1 && j < n - 1 ) {
                    (*sub)[i][j] = 0;
                }
            } 
        }
        for (size_t x = 0; x < n; x++){
            for (size_t y = 0; y < n; y++){
                size_t subi = 0;
                for (size_t i = 0; i < n; i++){
                    if (i == x) continue;
                    size_t subj = 0;
                    for (size_t j = 0; j < n; j++){
                        if (j == y) continue;
                        (*sub)[subi][subj] = m[i][j];
                        subj++;
                    }
                    subi++;
                }
                (*cof)[x][y] = ((x+y)%2==0?1:-1) * sub->DET();
            }
        }
    }

    double DET() const {
        if (n <= 0) {
            return 0;
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                (*det)[i][j] = m[i][j];
            }
        }

        double sign = 1;

        for(int k = 0; k < n - 1; k++) {
            //Pivot - row swap needed
            if((*det)[k][k] == 0) {
                int l = 0;
                for(l = k + 1; l < n; l++) {
                    if((*det)[l][k] != 0) {       
                        std::swap((*det)[l], (*det)[k]);
                        sign = -sign;
                        break;
                    }
                }

                // No entries != 0 found in column k -> det = 0
                if(l == n) {
                    return 0;
                }
            }

            //Apply formula
            for (int i = k + 1; i < n; i++) {
                for (int j = k + 1; j < n; j++) {
                    (*det)[i][j] = (*det)[k][k] * (*det)[i][j] - (*det)[i][k] * (*det)[k][j];
                    if(k != 0) {
                        (*det)[i][j] /= (*det)[k-1][k-1];
                    }
                }
            }
        }

        return sign * (*det)[n - 1][n - 1];
    }
    void INV() const {
        double det = DET();
        if (det == 0) throw MathError("Cannot find inverse(det = 0)");
        COF();
        T();
        t->multiply_matrix(1 / det);
    }
    
    void print() {
        for(auto& row:m){
            for (auto& elt : row) cout << elt << " ";
            cout << endl;
        }
    }
};


#endif