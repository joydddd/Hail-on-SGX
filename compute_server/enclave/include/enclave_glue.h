#ifndef ENCLAVE_H
#define ENCLAVE_H

//DEBUG:
/* For testrun without open enclave ONLY */

#include "gwas.h"
#include "server_type.h"

/* ECALL */
void setup_enclave(const int num_threads);
void log_regression(const int thread_id);


/* OCALLs */
void start_timer(const char func_name[ENCLAVE_READ_BUFFER_SIZE]);

void stop_timer(const char func_name[ENCLAVE_READ_BUFFER_SIZE]);

void setrsapubkey(uint8_t enc_rsa_pub_key[RSA_PUB_KEY_SIZE]);

void getclientnum(int* _retval);

void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]);

void getaes(bool* _retval, const int client_num, const int thread_id,
                   unsigned char key[256], unsigned char iv[256]);

void gety(int* _retval, const int client_num,
                 char y[ENCLAVE_READ_BUFFER_SIZE]);

void getcov(int* _retval, const int client_num,
                   const char cov_name[MAX_CLIENTNAME_LENGTH],
                   char cov[ENCLAVE_READ_BUFFER_SIZE]);

void get_encrypted_x_size(int* _retval, const int client_num);

void getbatch(int* _retval, char batch[ENCLAVE_READ_BUFFER_SIZE],
                     const int thread_id);

void writebatch(enum Row_T type, char buffer[ENCLAVE_OUTPUT_BUFFER_SIZE],
                       const int thread_id);

#endif