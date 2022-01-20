#ifndef ENC_BUFFER_H
#define ENC_BUFFER_H
#include <deque>

#include "enc_gwas.h"
#include "crypto.h"
#include "batch.h"

#ifdef NENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif

#define WORKING_THREAD_N 1
using std::deque;

class Batch;

struct ClientInfo{
    std::vector<AESData> aes_list;
    size_t crypto_size;
    size_t size;
};
void decrypt_batch(char* crypt, char* plaintxt, size_t* plaintxt_length,
                   const int thread_id, const int line_num);
void aes_decrypt_client(const unsigned char* crypto, unsigned char* plaintxt,
                        const ClientInfo& client, const int thread_id);
char* decrypt_line(char* crypt, char* plaintxt, size_t* plaintxt_length, const int thread_id);

class Buffer {
    /* meta data */
    size_t row_size;
    Row_T type;
    size_t output_tail;

    /* data member */
    char crypttxt[ENCLAVE_READ_BUFFER_SIZE];
    char output_buffer[ENCLAVE_OUTPUT_BUFFER_SIZE];

    /* Batch pool */
    deque<Batch*> free_batches;

    /* thread pool */

    void output(const char*);

public:
    Buffer(size_t _row_size, Row_T row_type);
    ~Buffer();
    void finish(Batch*);
    Batch* launch(const int thread_id);  // return nullptr if there is no free batches
};

#endif