#ifndef ENC_BUFFER_H
#define ENC_BUFFER_H
#include "enc_gwas.h"
#include <deque>


//DEBUG:
#define ENC_TEST

#ifdef ENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif

#define WORKING_THREAD_N 1
using std::deque;

void decrypt(char* crypt, char* plaintxt, size_t* plaintxt_length);

class Batch {
    /* data members */
    // char crypto[ENCLAVE_READ_BUFFER_SIZE]; // decrypt is handled by Buffer
    char plaintxt[ENCLAVE_READ_BUFFER_SIZE];
    size_t txt_size;

    /* status */
    size_t head = 0;
    enum Status { Empty, Working, Finished };

    /* working set */
    Row* row;

    /* meta data */
    size_t row_size;

   public:
    Batch(size_t _row_size) : row_size(_row_size) { row = new Row(row_size); }
    ~Batch() { delete row; }
    Status st = Empty;
    char* load_plaintxt() { return plaintxt; }
    size_t* plaintxt_size() { return &txt_size; }
    void reset();
    Row* get_row(); // return nullptr is reached ead of batch
};

class Buffer {
    /* meta data */
    size_t row_size;

    /* input data */
    char crypttxt[ENCLAVE_READ_BUFFER_SIZE];

    /* Batch pool */
    deque<Batch*> free_batches;
    
    /* thread pool */

    public:
     Buffer(size_t _row_size);
     ~Buffer();
     void finish(Batch*);
     Batch* launch(); // return nullptr if there is no free batches
};

#endif