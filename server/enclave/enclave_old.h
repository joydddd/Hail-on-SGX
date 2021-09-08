#ifndef ENCLAVE_H
#define ENCLAVE_H

#include "gwas.h"
#include "../server_type.h"

using namespace std;
void log_regression();
// void linear_regression_beta();
// void linear_regression_t_stat();

/* TODO */


/* enclave inoutput functions */
/* host function that can be called from enclave */
void writebatch(Row_T type, char buffer[ENCLAVE_OUTPUT_BUFFER_SIZE]);
// copy encrypted batch to host machine
// return nullptr while there is not batch avaible.


void getbatch(const char hostname[MAX_CLIENTNAME_LENGTH], Row_T type,
                     char batch[ENCLAVE_READ_BUFFER_SIZE]);
// get batch from outside of enclave
// return nullptr if the next batch hasn't arrived
// return const char* EndSperator if reaches end of dataset

void getclientlist(char hostlist[ENCLAVE_READ_BUFFER_SIZE]);
// copy hostlist from host machine to enclave

void gety(const char host[MAX_CLIENTNAME_LENGTH],
          char y[ENCLAVE_READ_BUFFER_SIZE]);
// copy y from host machine to enclave;

void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]);
// get covariantnumber from host

void getcov(const char host[MAX_CLIENTNAME_LENGTH],
            const char cov_name[MAX_CLIENTNAME_LENGTH],
            char cov[ENCLAVE_READ_BUFFER_SIZE]);
// copy no. covariant from host to enclave
//  "1" if the covariant is indent 1

#endif