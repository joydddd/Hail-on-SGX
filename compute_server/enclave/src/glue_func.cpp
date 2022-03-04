#include "enclave_glue.h"
#include "../../host/include/host_glue.h"

void getclientnum(int* _retval) { *_retval = getclientnum(); }

void getaes(bool* _retval, const int client_num, const int thread_id,
            unsigned char key[256], unsigned char iv[256]){
    *_retval = getaes(client_num, thread_id, key, iv);
}

void gety(int* _retval, const int client_num, 
    char y[ENCLAVE_READ_BUFFER_SIZE]){
    *_retval = gety(client_num, y);
}

void getcov(int* _retval, const int client_num,
            const char cov_name[MAX_CLIENTNAME_LENGTH],
            char cov[ENCLAVE_READ_BUFFER_SIZE]){
    *_retval = getcov(client_num, cov_name, cov);
}

void get_encrypted_x_size(int* _retval, const int client_num){
    *_retval = get_encrypted_x_size(client_num);
}

void getbatch(int* _retval, char batch[ENCLAVE_READ_BUFFER_SIZE],
              const int thread_id){
    *_retval = getbatch(batch, thread_id);
}