#ifndef ENCLAVE_H
#define ENCLAVE_H

#include "../gwas.h"
#include "type.h"

using namespace std;
void log_regression(bool intercept = false);
void linear_regression(bool intercept = false);

/* TODO */
// purly enclave function
extern void enclave_decrypt(char crypt[ENCLAVE_READ_BUFFER_SIZE],
                            string &plaintxt);

/* enclave inoutput functions */
/* host function that can be called from enclave */
extern void writebatch(Row_T type, char buffer[ENCLAVE_OUTPUT_BUFFER_SIZE]);
// copy encrypted batch to host machine
// return nullptr while there is not batch avaible.


extern void getbatch(const char hostname[MAX_HOST_LENGTH], Row_T type,
                     char batch[ENCLAVE_READ_BUFFER_SIZE]);
// get batch from outside of enclave
// return nullptr if the next batch hasn't arrived
// return const char* EndSperator if reaches end of dataset

extern void gethostlist(char hostlist[ENCLAVE_READ_BUFFER_SIZE]);
// copy hostlist from host machine to enclave

extern void gety(const char host[MAX_HOST_LENGTH],
                 char y[ENCLAVE_READ_BUFFER_SIZE]);
// copy y from host machine to enclave;

extern void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]);
// get covariantnumber from host

extern void getcov(const char host[MAX_HOST_LENGTH], const char cov_name[MAX_HOST_LENGTH],
            char cov[ENCLAVE_READ_BUFFER_SIZE]);
// copy no. covariant from host to enclave
//  "1" if the covariant is indent 1

#endif