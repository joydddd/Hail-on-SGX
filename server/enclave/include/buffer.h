#ifndef ENC_BUFFER_H
#define ENC_BUFFER_H
#include <deque>

#include "enc_gwas.h"
#include "crypto.h"

#ifdef NENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif

#define WORKING_THREAD_N 1
using std::deque;

struct ClientInfo{
    AESData aes;
    size_t crypto_size;
    size_t size;
};

extern vector<ClientInfo> clientinfo;

void decrypt(char* crypt, char* plaintxt, size_t* plaintxt_length);

class Batch {
    /* data members */
    // char crypto[ENCLAVE_READ_BUFFER_SIZE]; // decrypt is handled by Buffer
    char plaintxt[ENCLAVE_READ_BUFFER_SIZE];
    size_t txt_size;
    Row_T type;
    char outtxt[ENCLAVE_OUTPUT_BUFFER_SIZE];

    /* status */
    size_t head = 0;
    size_t out_tail = 0;

    /* working set */
    Row* row;

    /* meta data */
    size_t row_size;

   public:
    Batch(size_t _row_size, Row_T row_type);
    ~Batch() { delete row; }

    /* status */
    enum Status { Empty, Working, Finished };
    Status st = Empty;

    char* load_plaintxt() { return plaintxt; }
    const char *output_buffer() { return outtxt; }
    size_t *plaintxt_size() { return &txt_size; }
    void reset();
    Row* get_row();  // return nullptr is reached ead of batch
    void write(const string &);
};

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
    Batch* launch();  // return nullptr if there is no free batches
};
extern Buffer* buffer;

#endif