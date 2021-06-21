#include <vector>
#include <string>
#include <map>

#define NA -1
class ERROR_t {
    public:
    ERROR_t(std::string _msg):msg(_msg){}
    std::string msg;
};

class ReadtsvERROR : public ERROR_t{
    using ERROR_t::ERROR_t;
};
class mathERROR : public ERROR_t{
    using ERROR_t::ERROR_t;
};
class exportERROR : public ERROR_t{
    using ERROR_t::ERROR_t;
};

class GWAS_row {
   public:
    std::vector<double> data;
    void read_row(std::string &line, std::string &locu, std::string &allele);
    void read_col_ref(std::ifstream &fs, std::string &key, const std::vector<std::string> &ref);
    void read_col(std::ifstream &fs, std::string &key,
                  std::vector<std::string> &col_keys);
    double dot_mult(const GWAS_row &multiplier,
                 const std::vector<double> &valid) const;
    void XTX(std::vector<GWAS_row>::iterator co_begin, std::vector<GWAS_row>::iterator co_end, std::vector<double>::iterator ans, const GWAS_row &NA_ref) const;
    void XTY(GWAS_row &y, std::vector<GWAS_row> &covariants,
             std::vector<double> &ans) const;
    size_t size() const { return data.size(); }
    std::pair<double, int> SSE(std::vector<double> &beta, std::vector<GWAS_row> &covariants, GWAS_row &y);
};

class GWAS{
    private:
     std::vector<std::string> col_keys;
     std::vector<std::string> locus;
     std::vector<std::string> alleles;
     std::vector<GWAS_row> data;
     GWAS_row y;
     std::string y_key;
     std::vector<GWAS_row> covariant;
     std::vector<std::string> covariant_key;

    public:
     GWAS(std::string &y_file, std::vector<std::string> &cov_files);
     void read_alleles(std::string &filename); // accept NA entries
     void read_y(std::string &filename); // doesn't accept NA entries -- filter out before exporting 
     void read_covariant(std::string &filename); // doens't accept NA entries --filter out before exporting
     void export_XTX(std::string &filename);
     void export_XTY(std::string &filename);
     void export_SSE(std::map<std::string, std::vector<double>> &beta, std::string &filename);
     void print();
};