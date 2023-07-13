#ifndef ENCLAVE_H
#define ENCLAVE_H

//DEBUG:
/* For testrun without open enclave ONLY */

#include "gwas.h"
#include "server_type.h"
#include <limits>

/* ECALL */
void setup_enclave_encryption(const int num_threads);
void setup_num_patients();
void setup_enclave_phenotypes(const int num_threads, enum EncAnalysis analysis_type, enum ImputePolicy impute_policy);
void regression(const int thread_id, EncAnalysis analysis_type);

/* OCALLs */
void start_timer(const char func_name[ENCLAVE_READ_BUFFER_SIZE]);

void stop_timer(const char func_name[ENCLAVE_READ_BUFFER_SIZE]);

void setrsapubkey(uint8_t enc_rsa_pub_key[RSA_PUB_KEY_SIZE]);

void setmaxbatchlines(int lines);

void getclientnum(int* _retval);

void get_num_patients(int* _retval, const int client_num, char num_patients_buffer[ENCLAVE_SMALL_BUFFER_SIZE]);

void getcovlist(char covlist[ENCLAVE_READ_BUFFER_SIZE]);

void getaes(bool* _retval, const int client_num, const int thread_id,
                   unsigned char key[256], unsigned char iv[256]);

void gety(int* _retval, const int client_num,
                 char y[ENCLAVE_READ_BUFFER_SIZE]);

void getcov(int* _retval, const int client_num,
                   const char cov_name[MAX_CLIENTNAME_LENGTH],
                   char cov[ENCLAVE_READ_BUFFER_SIZE]);

void getbatch(int* _retval, char batch[ENCLAVE_READ_BUFFER_SIZE],
                     const int thread_id);

void writebatch(char buffer[ENCLAVE_READ_BUFFER_SIZE],
                    const int buffer_size,
                    const int thread_id);

#endif