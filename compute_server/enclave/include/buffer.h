#ifndef ENC_BUFFER_H
#define ENC_BUFFER_H

#include "enc_gwas.h"
#include "crypto.h"
#include "batch.h"
#include <fstream>

#ifdef NON_OE
#include "enclave_glue.h"
#else
#include "gwas_t.h"
#endif

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

    Batch* free_batch;

    int* client_list;
    char** client_crypto_map;
    int client_count;

    int thread_id;

    void output(const char* out, const size_t& length);

    void decrypt_line(char* plaintxt, size_t* plaintxt_length, unsigned int num_lines, const std::vector<ClientInfo>& client_info_list, const int thread_id);

public:
    Buffer(GWAS* _gwas, size_t _row_size, Row_T row_type, int num_clients, int thread_id);
    ~Buffer();
    void finish();
    void clean_up();

    Batch* launch(std::vector<ClientInfo>& client_info_list, const int thread_id);  // return nullptr if there is no free batches
};

#endif