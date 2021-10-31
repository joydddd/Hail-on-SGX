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
    SqrMatrix(size_t s):m(s, vector<double>(s,0)), n(s){}
    SqrMatrix(vector<vector<double>> &vec):m(vec), n(vec.size()){
        if (n > 0 && m[0].size() != n)
            throw MathError("SqrMatrix: not a square matrix");
    }
    SqrMatrix(const SqrMatrix&) = default;
    SqrMatrix(SqrMatrix&&) = default;
    virtual ~SqrMatrix() = default;
    SqrMatrix& operator=(const SqrMatrix&) = default;
    SqrMatrix& operator=(SqrMatrix&) = default;
    vector<double>& operator[](size_t index){return m[index];}
    double at(size_t row, size_t col) const {return m[row][col];}
    size_t size() const {return n;}
    bool is_empty() { return n == 0; }
    friend SqrMatrix operator+(const SqrMatrix& me, const SqrMatrix &other) {
        if (me.n != other.n) throw MathError("SqrMatrix size mistmatch");
        SqrMatrix sum(me.n);
        for (size_t i=0; i<me.n; i++){
            for (size_t j=0; j<me.n; j++){
                sum.m[i][j] = me.m[i][j] + other.m[i][j];
            }
        }
        return {sum};
    }
    friend SqrMatrix operator*(const SqrMatrix& me, double mult){
        SqrMatrix ans(me.n);
        for (size_t i=0; i<me.n; i++){
            for (size_t j=0; j<me.n; j++){
                ans[i][j] = me.m[i][j] * mult;
            }
        }
        return ans;
    }
    friend vector<double> operator*(const SqrMatrix& me, const vector<double>& mult){
        if (mult.size() != me.n) throw MathError("Matrix Vector dim mismatch");
        vector<double> ans(me.n, 0);
        for (size_t i=0; i<me.n; i++){
            for (size_t j=0; j<me.n; j++){
                ans[i] += me.m[i][j] * mult[j];
            }
        }
        return ans;
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
    SqrMatrix T(){
        SqrMatrix t(n);
        for (size_t i=0; i<n; i++){
            for (size_t j=0; j<n; j++){
                t[i][j] = m[j][i];
            }
        }
        return t;
    }

    vector<double> DIAG()const{
        vector<double> ans;
        ans.resize(n);
        for(size_t i=0; i<n; i++){
            ans[i] = m[i][i];
        }
        return ans;
    }
    
    SqrMatrix COF() const {
        if (n==1) return 1;
        SqrMatrix cof(n);
        SqrMatrix sub(n-1);
        for (size_t x=0; x<n; x++){
            for (size_t y=0; y<n; y++){
                size_t subi = 0;
                for (size_t i=0; i<n; i++){
                    if (i==x) continue;
                    size_t subj=0;
                    for (size_t j=0; j<n; j++){
                        if (j==y) continue;
                        sub[subi][subj] = m[i][j];
                        subj++;
                    }
                    subi++;
                }
                cof[x][y] = ((x+y)%2==0?1:-1)*sub.DET();
            }
        }
        return cof;
    }
    double DET() const {
        size_t n = m.size();
        double D=0;
        if (n==1) return m[0][0];
        if (n==2) return m[0][0] * m[1][1] - m[1][0] * m[0][1];
        SqrMatrix sub(n-1);
        for (size_t x=0; x<n; x++){
            size_t subi = 0;
            for (size_t i=1; i<n; i++){
                size_t subj=0;
                for (size_t j=0; j<n; j++){
                    if (j==x) continue;
                    sub[subi][subj] = m[i][j];
                    subj++;
                }
                subi++;
            }
            D = D + (x%2==0?1:-1) * m[0][x] * sub.DET();
        }
        return D;
    }
    SqrMatrix INV() const {
        double det = DET();
        if (det == 0) throw MathError("Cannot find inverse(det = 0)");
        return (COF().T())*(1/det);
    }
    
};


#endif