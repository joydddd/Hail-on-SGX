#ifndef BUFFER_SIZE_H
#define BUFFER_SIZE_H


#define ENCLAVE_OUTPUT_BUFFER 10  // in KB
#define ENCLAVE_READ_BUFFER 15    // in KB
#define MAX_CLIENTNAME_LENGTH 30
#define BUFFER_UPDATE_INTERVAL 10  // in milliseconds
#define AES_KEY_LENGTH 16 // 128 bit enc
#define AES_IV_LENGTH 16 // 128 bit enc

#define ENCLAVE_OUTPUT_BUFFER_SIZE ENCLAVE_OUTPUT_BUFFER * 1024  // in B

#define ENCLAVE_READ_BUFFER_SIZE ENCLAVE_READ_BUFFER * 1024  // in B

#define RSA_PUB_KEY_SIZE 512

#define EOFSeperator "~EOF~" // mark end of dataset

#endif



