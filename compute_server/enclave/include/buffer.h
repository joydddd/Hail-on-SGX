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

void aes_decrypt_client(const unsigned char* crypto, unsigned char* plaintxt, const ClientInfo& client, const int thread_id);
void two_bit_decompress(uint8_t* input, uint8_t* decompressed, unsigned int size);

class Buffer {
    /* meta data */
    size_t row_size;
    Row_T type;
    size_t output_tail;

    /* data member */
    char crypttxt[ENCLAVE_READ_BUFFER_SIZE];
    uint8_t plain_txt_compressed[ENCLAVE_READ_BUFFER_SIZE];
    char output_buffer[ENCLAVE_OUTPUT_BUFFER_SIZE];

    /* Batch pool */
    deque<Batch*> free_batches;

    int* client_list;
    char** client_crypto_map;
    int client_count;

    /* thread pool */

    void output(const char*);

    void decrypt_line(char* plaintxt, size_t* plaintxt_length, const std::vector<ClientInfo>& client_info_list, const int thread_id);

public:
    Buffer(size_t _row_size, Row_T row_type, int num_clients);
    ~Buffer();
    void finish(Batch*);
    Batch* launch(std::vector<ClientInfo>& client_info_list, const int thread_id);  // return nullptr if there is no free batches
};

#endif