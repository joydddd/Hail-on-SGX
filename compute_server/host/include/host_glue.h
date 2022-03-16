#include "server_type.h"
#include "gwas.h"
/* OCALL */
int getclientnum();

bool getaes(const int client_num, const int thread_id, unsigned char key[256],
            unsigned char iv[256]);
int gety(const int client_num, char y[ENCLAVE_READ_BUFFER_SIZE]);
int getcov(const int client_num, const char cov_name[MAX_CLIENTNAME_LENGTH],
           char cov[ENCLAVE_READ_BUFFER_SIZE]);
int get_encrypted_x_size(const int client_num);
int getbatch(char batch[ENCLAVE_READ_BUFFER_SIZE], const int thread_id);