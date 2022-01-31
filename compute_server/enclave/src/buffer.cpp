#include "batch.h"
#include "buffer.h"

#include "logistic_regression.h"
#include "string.h"
#include <map>

void aes_decrypt_client(const unsigned char* crypto, unsigned char* plaintxt, const ClientInfo& client, const int thread_id){
    aes_decrypt_data(client.aes_list[thread_id].aes_context,
                     (unsigned char *)client.aes_list[thread_id].aes_iv,
                     crypto,
                     client.crypto_size, 
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

void Buffer::decrypt_line(char* plaintxt, size_t* plaintxt_length, const std::vector<ClientInfo>& client_info_list, const int thread_id) {
    vector<char*> client_begin;
    char* head = crypttxt;
    char* end_of_allele = crypttxt, *end_of_loci = crypttxt;
    while (true) {
        if (*head == '\t') {
            if (end_of_allele != crypttxt) {
                end_of_loci = head;
                break;
            } else {
                end_of_allele = head;
            }
        }
        head++;
    }
    /* copy allele & loci to plaintxt */
    char* plaintxt_head = plaintxt;
    strncpy(plaintxt, crypttxt, end_of_loci - crypttxt + 1);
    plaintxt_head += end_of_loci - crypttxt + 1;

    char* tab_pos = end_of_loci;
    deque<int> client_list;
    /* get client list */
    head++;
    while (true) {
        if(*head == '\t'){
            *head = '\0';
            int client = atoi(tab_pos + 1);
            client_list.push_back(client);
            // cout << "head = " << head - crypt
            //      << " tab_pos = " << tab_pos - crypt << endl;
            // cout << "client: " << client << endl;
            *head = '\t';
            tab_pos = head;
        }
        if (*head == ' ') {
            *head = '\0';
            int client = atoi(tab_pos + 1);
            client_list.push_back(client);
            // cout << "head = " << head - crypt
            //      << " tab_pos = " << tab_pos - crypt << endl;
            // cout << "client: " << client << endl;
            *head = ' ';
            head++;
            break;
        }
        head++;
    }

    /* decrypt data */
    map<int, char*> client_crypto_map;
    for (size_t i = 0; i < client_list.size(); i++){
        client_crypto_map[client_list[i]] = head;
        head += client_info_list[client_list[i]].crypto_size;
    }
    for (int client = 0; client < client_info_list.size(); client++) {
        if (client_crypto_map.find(client) == client_crypto_map.end()){  // this client does have target allele
            for (int j = 0; j < client_info_list[client].size; j++)
                *(plaintxt_head + j) = NA_uint8;
        } else {
            aes_decrypt_client((const unsigned char*)client_crypto_map[client],
                                (unsigned char*)plain_txt_compressed,
                                client_info_list[client], thread_id);
            two_bit_decompress(plain_txt_compressed, (uint8_t*)plaintxt_head, client_info_list[client].size);
        }
        plaintxt_head += client_info_list[client].size;
    }
    *plaintxt_head = '\n';
    plaintxt_head++;
    *plaintxt_head = '\0';
    *plaintxt_length = plaintxt_head - plaintxt;
}

Buffer::Buffer(size_t _row_size, Row_T row_type)
    : row_size(_row_size), type(row_type) {
    for (size_t i = 0; i < WORKING_THREAD_N; i++) {
        free_batches.push_back(new Batch(row_size, type));
    }
}

Buffer::~Buffer() {
    for (Batch* bat : free_batches) {
        delete bat;
    }
}

void Buffer::output(const char* out) {
    int len = strlen(out);
    if (output_tail + len >= ENCLAVE_OUTPUT_BUFFER) {
        writebatch(type, output_buffer);
        output_tail = 0;
    }
    strcpy(output_buffer + output_tail, out);
    output_tail += strlen(out);
}

void Buffer::finish(Batch* finishing_batch) {
    output(finishing_batch->output_buffer());
    finishing_batch->reset();
    free_batches.push_back(finishing_batch);
}

Batch* Buffer::launch(std::vector<ClientInfo>& client_info_list, const int thread_id) {
    int rt = 0;
    while (!rt) {
        getbatch(&rt, crypttxt, thread_id);
    }
    if (!strcmp(crypttxt, EOFSeperator)) return nullptr;
    if (free_batches.empty()) return nullptr;
    Batch* new_b = free_batches.front();
    free_batches.pop_front();
    decrypt_line(new_b->load_plaintxt(), new_b->plaintxt_size(), client_info_list, thread_id);
    return new_b;
}