#ifndef BATCH_H
#define BATCH_H

#include "buffer.h"
#include "logistic_regression.h"
#include "enc_gwas.h"

#ifdef NON_OE
#include "enclave_glue.h"
#else
#include "gwas_t.h"
#endif

class Buffer;

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
    Row* get_row(Buffer* buffer);  // return nullptr is reached ead of batch
    void write(const string &);
};

#endif