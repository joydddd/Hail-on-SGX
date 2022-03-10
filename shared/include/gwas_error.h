#ifndef GWAS_ERROR_H
#define GWAS_ERROR_H
#include <string>

;

class ERROR_t {
   public:
    ERROR_t(std::string _msg) : msg(_msg) {}
    std::string msg;
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