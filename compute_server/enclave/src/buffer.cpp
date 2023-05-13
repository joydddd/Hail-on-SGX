#include "batch.h"
#include "buffer.h"

#include "logistic_regression.h"
#include "string.h"
#include <map>

void aes_decrypt_client(const unsigned char* crypto, unsigned char* plaintxt, const ClientInfo& client, const int thread_id) {
    aes_decrypt_data(client.aes_list[thread_id].aes_context,
                     (unsigned char *)client.aes_list[thread_id].aes_iv,
                     crypto,
                     client.crypto_size - 1, 
                     plaintxt);
}

void two_bit_decompress(uint8_t* input, uint8_t* decompressed, unsigned int size) {
    int input_idx = 0;
    int two_bit_arr_count = 0;
    for (int decompressed_idx = 0; decompressed_idx < size; ++decompressed_idx) {
        decompressed[decompressed_idx] = input[input_idx] & 0b11;
        if (++two_bit_arr_count == TWO_BIT_INT_ARR_SIZE) {
            input_idx++;
            two_bit_arr_count = 0;
        } else {
            input[input_idx] >>= 2;
        }
    }
}

void Buffer::decrypt_line(char* plaintxt, size_t* plaintxt_length, unsigned int num_lines, const std::vector<ClientInfo>& client_info_list, const int thread_id) {
    char* crypt_head = crypttxt; 
    char *crypt_start, *end_of_allele, *end_of_loci;
    char* plaintxt_head = plaintxt;
    for (int line = 0; line < num_lines; ++line) {
        crypt_start = crypt_head;
        end_of_allele = crypt_head;
        end_of_loci = crypt_head;
        while (true) {
            if (*crypt_head == '\t') {
                if (end_of_loci != crypt_start) {
                    end_of_allele = crypt_head;
                    break;
                } else {
                    end_of_loci = crypt_head;
                }
            }
            crypt_head++;
        }
        /* copy allele & loci to plaintxt */
        strncpy(plaintxt_head, crypt_start, end_of_allele - crypt_start + 1);
        plaintxt_head += end_of_allele - crypt_start + 1;

        char* tab_pos = end_of_allele;
        client_count = 0;
        /* get client list */
        crypt_head++;
        while (true) {
            if(*crypt_head == '\t'){
                *crypt_head = '\0';
                int client = atoi(tab_pos + 1);
                client_list[client_count++] = client;
                *crypt_head = '\t';
                tab_pos = crypt_head;
            }
            if (*crypt_head == ' ') {
                *crypt_head = '\0';
                int client = atoi(tab_pos + 1);
                client_list[client_count++] = client;
                *crypt_head = ' ';
                crypt_head++;
                break;
            }
            crypt_head++;
        }
        /* decrypt data */
        for (size_t i = 0; i < client_count; i++){
            client_crypto_map[client_list[i]] = crypt_head;
            crypt_head += client_info_list[client_list[i]].crypto_size;
        }
        bool client_found;
        for (int client = 0; client < client_info_list.size(); client++) {
            client_found = false;
            for (int list_id = 0; list_id < client_count; ++list_id) {
                if (client_list[list_id] == client) {
                    aes_decrypt_client((const unsigned char*)client_crypto_map[list_id],
                                       (unsigned char*)plaintxt_head,
                                       client_info_list[client], 
                                       thread_id);
                    // two_bit_decompress(plain_txt_compressed, 
                    //                    (uint8_t*)plaintxt_head, 
                    //                    client_info_list[client].size);
                    // memset(plain_txt_compressed, 0, ENCLAVE_READ_BUFFER_SIZE);
                    // memset(client_crypto_map[list_id], 0, client_info_list[client].crypto_size);
                    client_found = true;
                }
            }
            if (!client_found) {
                // this client does have target allele
                for (int j = 0; j < client_info_list[client].size; j++) {
                    *(plaintxt_head + j) = NA_uint8;
                }
            }
            plaintxt_head += client_info_list[client].size;
            //plaintxt_head += (client_info_list[client].size / 4) + (client_info_list[client].size % 4 == 0 ? 0 : 1);
        }
        *plaintxt_head = '\n';
        plaintxt_head++;
    }
    *plaintxt_head = '\0';
    *plaintxt_length = plaintxt_head - plaintxt;
}

Buffer::Buffer(size_t _row_size, EncAnalysis type, int num_clients, int _thread_id)
    : row_size(_row_size), analysis_type(type), thread_id(_thread_id) {
    crypttxt = new char[ENCLAVE_READ_BUFFER_SIZE];
    plain_txt_compressed = new uint8_t[ENCLAVE_READ_BUFFER_SIZE];
    client_list = new int[num_clients];
    client_crypto_map = new char* [num_clients];
    // I now remember why we do this! Because we do batching, we can load in ENCLAVE_READ_BUFFER_SIZE
    // amount of data in at a time, BUT this data when decompressed can actually be up to 4 * ENCLAVE_READ_BUFFER_SIZE large
    plaintxt_buffer = new char[ENCLAVE_READ_BUFFER_SIZE];
    output_tail = 0;

    memset(crypttxt, 0, ENCLAVE_READ_BUFFER_SIZE);
    memset(plain_txt_compressed, 0, ENCLAVE_READ_BUFFER_SIZE);
    memset(plaintxt_buffer, 0, ENCLAVE_READ_BUFFER_SIZE);
}

Buffer::~Buffer() {
    delete free_batch;
    delete [] client_list;
    delete [] client_crypto_map;
}

void Buffer::add_gwas(GWAS* _gwas, ImputePolicy impute_policy) {
    free_batch = new Batch(row_size, analysis_type, impute_policy, _gwas, plaintxt_buffer);
}

void Buffer::output(const char* out, const size_t& length) {
    if (output_tail + length >= ENCLAVE_READ_BUFFER_SIZE) {
        writebatch(output_buffer, output_tail, thread_id);
        memset(output_buffer, 0, ENCLAVE_READ_BUFFER_SIZE);
        output_tail = 0;
    }
    strcpy(output_buffer + output_tail, out);
    output_tail += length;
}

void Buffer:: clean_up() {
    if (output_tail > 0) {
        writebatch(output_buffer, output_tail, thread_id);
    }
}

void Buffer::finish() {
    output(free_batch->output_buffer(), free_batch->get_out_tail());
    free_batch->reset();
}

Batch* Buffer::launch(std::vector<ClientInfo>& client_info_list, const int thread_id) {
    int num_lines = 0;
    while (!num_lines) {
        getbatch(&num_lines, crypttxt, thread_id);
    }
    if (!strcmp(crypttxt, EOFSeperator)) return nullptr;
    if (!free_batch) return nullptr;
    *free_batch->plaintxt_size() = 0;
    decrypt_line(free_batch->load_plaintxt(), free_batch->plaintxt_size(), num_lines, client_info_list, thread_id);
    return free_batch;
}