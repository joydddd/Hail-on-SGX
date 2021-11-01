#ifndef GWAS_ERROR_H
#define GWAS_ERROR_H
#include <string>

using namespace std;

class ERROR_t {
   public:
    ERROR_t(string _msg) : msg(_msg) {}
    string msg;
};

class ReadtsvERROR : public ERROR_t {
    using ERROR_t::ERROR_t;
};
class CombineERROR : public ERROR_t {
    using ERROR_t::ERROR_t;
};

class ENC_ERROR : public ERROR_t {
    using ERROR_t::ERROR_t;
};

class mathERROR : public ERROR_t {
    using ERROR_t::ERROR_t;
};
class exportERROR : public ERROR_t {
    using ERROR_t::ERROR_t;
};

#endif