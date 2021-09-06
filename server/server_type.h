#ifndef TYPE_SERVER_H
#define TYPE_SERVER_H

#define ENCLAVE_OUTPUT_BUFFER 10  // in KB
#define ENCLAVE_READ_BUFFER 10    // in KB
#define MAX_HOST_LENGTH 30
#define BUFFER_UPDATE_INTERVAL 10  // in milliseconds

#define ENCLAVE_OUTPUT_BUFFER_SIZE ENCLAVE_OUTPUT_BUFFER * 1024  // in B

#define ENCLAVE_READ_BUFFER_SIZE ENCLAVE_READ_BUFFER * 1024  // in B
/* enclave inoutput header */
enum Row_T { XTX_t, XTY_t, SSE_t, LOG_t, BETA_t, Result_t };

#endif