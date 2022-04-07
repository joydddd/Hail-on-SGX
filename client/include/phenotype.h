/*
 * Header file for our phenotype class.
 */


#ifndef _PHENOTYPE_H_
#define _PHENOTYPE_H_

#include <string>

#include "parser.h"

struct Phenotype {
    std::string message;
    ComputeServerMessageType mtype;
};

// class Phenotype {
//   private:
//     std::string message;
//     ComputeServerMessageType mtype;

//   public:
//     Phenotype(std::string _message, ComputeServerMessageType _mtype);
//     ~Phenotype();

//     std::string get_message();

//     ComputeServerMessageType get_mtype();

// };

#endif /* _PHENOTYPE_H*/
