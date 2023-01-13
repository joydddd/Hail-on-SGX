#include "server_type.h"
#include "gwas.h"
/* OCALL */
int getclientnum();

bool getaes(const int client_num, const int thread_id, unsigned char key[256],
            unsigned char iv[256]);
int get_num_patients(const int client_num, char num_patients_buffer[ENCLAVE_SMALL_BUFFER_SIZE]);
int gety(const int client_num, char y[ENCLAVE_READ_BUFFER_SIZE]);
int getcov(const int client_num, const char cov_name[MAX_CLIENTNAME_LENGTH],
           char cov[ENCLAVE_READ_BUFFER_SIZE]);
int getbatch(char batch[ENCLAVE_READ_BUFFER_SIZE], const int thread_id);