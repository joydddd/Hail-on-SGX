#ifndef BUFFER_SIZE_H
#define BUFFER_SIZE_H

/* 
Only modify the buffer size header file in the shared directory!
The Makefile will copy that header to another location for the build process.
*/

#define ENCLAVE_READ_BUFFER 2000    // in KB
#define ENCLAVE_SMALL_BUFFER 10 // in KB
#define MAX_CLIENTNAME_LENGTH 30
#define AES_KEY_LENGTH 16 // 128 bit enc
#define AES_IV_LENGTH 16 // 128 bit enc

#define ENCLAVE_READ_BUFFER_SIZE ENCLAVE_READ_BUFFER * 1024  // in B

#define ENCLAVE_SMALL_BUFFER_SIZE ENCLAVE_SMALL_BUFFER * 1024

#define RSA_PUB_KEY_SIZE 512

#define TWO_BIT_INT_ARR_SIZE 4 // compressed two bit uint8_t array size (8 bits / 2 bits per value)

#define MAX_LOCI_ALLELE_STR_SIZE 28

#define EOFSeperator "~EOF~" // mark end of dataset

enum EncAnalysis { linear, logistic };
enum ImputePolicy { EPACTS, Hail };

#endif



