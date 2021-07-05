#include <vector>
#include <string>
#include <map>
#include <iostream>

using namespace std;

#define NA -1
#define X 23
class ERROR_t
{
public:
    ERROR_t(string _msg) : msg(_msg) {}
    string msg;
};

class ReadtsvERROR : public ERROR_t
{
    using ERROR_t::ERROR_t;
};
class mathERROR : public ERROR_t
{
    using ERROR_t::ERROR_t;
};
class exportERROR : public ERROR_t
{
    using ERROR_t::ERROR_t;
};
class alleleERROR : public ERROR_t
{
    using ERROR_t::ERROR_t;
};
enum ALLELE : char
{
    A='A',
    T='T',
    C='C',
    G='G',
    NAN='N'
};


class Loci
{
public:
    int chrom;
    int loc;
    Loci(){};
    Loci(string &str);
    friend ostream& operator<<(ostream& os, const Loci &loci){
        if (loci.chrom == X) os << "X";
        else os << loci.chrom;
        os << ":" << (loci.loc);
        return os;
    }
    friend bool operator<(const Loci& a, const Loci& b){
        if (a.chrom < b.chrom ) return true;
        if (a.chrom > b.chrom ) return false;
        return a.loc < b.loc;
    }
    friend bool operator==(const Loci& a, const Loci& b){
        return a.loc == b.loc && a.chrom == b.chrom;
    }
    friend bool operator>(const Loci&a, const Loci&b){
        return !( a<b || a==b);
    }
    string str();
};


class Alleles
{
public:
    ALLELE a1, a2;
    Alleles() : a1(NAN), a2(NAN) {}
    bool read(string str); // return true if the allele is inversed.
    friend ostream& operator<<(ostream &os, const Alleles &alleles){
        os << "[\"" << (char)alleles.a1 << "\",\""<< (char)alleles.a2 << "\"]";
        return os;
    }
    friend bool operator==(const Alleles &a, const Alleles &b){
        return a.a1 == b.a1 && a.a2 == b.a2;}
};

class GWAS_var
{
    vector<vector<double>> XTX_xi;
    vector<double> XTY_x;
    bool prepared_XTX = false, prepared_XTY = false;
    void mult(const vector<double> &x, vector<double> &ans);
    void prepare_XTX(vector<GWAS_var>::iterator me, vector<GWAS_var>::iterator end);
    void prepare_XTY();
public:
    vector<double> data;
    GWAS_var* y;
    size_t size() const { return data.size(); }
    void build_1(size_t length);
    void read(ifstream &fs, string &key, vector<string> &col_keys);
    void read_ref(ifstream &fs, string &key, const vector<string> &ref);


    double dot_mult(const vector<int> &x) const;
    void print_XTX(ostream &fs, const vector<int> &NA_ref, vector<GWAS_var>::iterator me, vector<GWAS_var>::iterator end);
    void print_XTY(ostream &fs, const vector<int> &NA_ref);
    
};

class GWAS_row
{
    double X2() const;
public:
    Loci loci;
    Alleles alleles;
    vector<int> data;
    GWAS_row(string &line); 
    // construct row from parts. for SNPs
    void inverse();                            
    // for SNPs. in case of major minor allele swap.
    size_t size() const { return data.size(); }

    void print_XTX(ostream &fs, vector<GWAS_var> &covariants) const;
    void print_XTY(ostream &fs, GWAS_var &y, vector<GWAS_var> &covariants) const;

    void print_SSE(ostream &fs, vector<double> &beta, vector<GWAS_var> &covariants, GWAS_var &y);

    void print();
};

class Beta_row{
    public:
    Loci loci;
    Alleles alleles;
    vector<double> beta;
    Beta_row(vector<string> &parts, string &loci_str, string &allele_str);
};


class GWAS
{
private:
    vector<string> col_keys;
    GWAS_var y;
    string y_key;
    vector<GWAS_var> covariant;
    vector<string> covariant_key;

    /* SNP data */
    vector<GWAS_row> data;

    void read_y(string filename);         // doesn't accept NA entries -- filter out before exporting
    void read_covariant(string filename); // doens't accept NA entries --filter out before exporting

    /* sse iterator */
    vector<GWAS_row>::iterator sse_it;

public:
    GWAS(string y_file, vector<string> &cov_files);
    void read_alleles(string filename); // accept NA entries
    void clear_rows() { data.clear(); }
    void print_XTX(ostream &fs);
    void print_XTX_title(ostream &fs);
    void print_XTY(ostream &fs);
    void print_XTY_title(ostream &fs);
    void print_SSE_title(ostream &fs) { fs << "locus\talleles\tn\tSSE" << endl; }
    void print_SSE(ostream &fs, vector<Beta_row> &betas);
    void SSE_init(){sse_it = data.begin();}
    void print();
};

void export_XTX_XTY(GWAS &gwas, string XTX_filename, string XTY_filename);
void export_SSE(GWAS &gwas, string beta_filename, string SSE_filename);