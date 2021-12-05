#include "buffer.h"

#include "logistic_regression.h"
#include "string.h"

void decrypt(int client, char* crypto, char* plaintxt){
    cout << "PLAIN CRYPTO:" << crypto << endl;
    aes_decrypt_data(clientinfo[client].aes.aes_context,
                     clientinfo[client].aes.aes_iv,
                     (const unsigned char*)crypto,
                     clientinfo[client].crypto_size, (unsigned char*)plaintxt);
    cout << "PLAIN TXT" << plaintxt << endl;
}

void decrypt_line(char* crypt, char* plaintxt, size_t* plaintxt_length) {
    vector<char*> client_begin;
    char* head = crypt;
    char* end_of_allele = crypt, *end_of_loci = crypt;
    while (true) {
        if (*head == '\t') {
            if (end_of_allele != crypt) {
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
    strncpy(plaintxt, crypt, end_of_loci - crypt + 1);
    plaintxt_head += end_of_loci - crypt + 1;
    printf("Alelle & Loci %s\n", plaintxt);

    char* tab_pos = end_of_loci;
    deque<int> client_list;
    /* get client list */
    while(true) {
        if(*head == '\t'){
            *head == '\0';
            int client = atoi(tab_pos + 1);
            client_list.push_back(client);
            cout << "client: " << client << endl;
            *head = '\t';
            tab_pos = head;
        }
        if (*head == ' ') {
            head++;
            break;
        }
        head++;
    }
    

    /* decrypt data */
    for (int i = 0; i < clientinfo.size(); i++){
        if (i != client_list.front()) { // this client does have target allele
            for(int j=0; j<clientinfo[i].size; j++)
                *(plaintxt_head + j) = NA_uint8;
    
        } else {
            decrypt(i, head, plaintxt_head);
            head += clientinfo[i].crypto_size;
        }
        plaintxt_head + clientinfo[i].size;
    }
    *plaintxt_head = '\n';
    plaintxt_head++;
    *plaintxt_head = '\0';
    *plaintxt_length = plaintxt_head - plaintxt;
}

Batch::Batch(size_t _row_size, Row_T row_type)
    : row_size(_row_size), type(row_type) {
    switch (type) {
        case LOG_t:
            row = new Log_row(row_size);
            break;
        default:
            row = new Row(row_size);
            break;
    }
}

void Batch::reset() {
    head = 0;
    st = Empty;
    txt_size = 0;
    out_tail = 0;
}

Row* Batch::get_row() {
    if (head >= txt_size) {
        st = Finished;
        buffer->finish(this);
        return nullptr;
    }
    st = Working;
    row->reset();
    head += row->read(plaintxt + head);
#ifdef DEBUG
    // row->print();
#endif
    return row;
}

void Batch::write(const string& output) {
    strcpy(outtxt + out_tail, output.c_str());
    out_tail += output.size();
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

Batch* Buffer::launch() {
    bool rt;
    getbatch(&rt, crypttxt);
    if (!strcmp(crypttxt, EOFSeperator)) return nullptr;
    if (free_batches.empty()) return nullptr;
    Batch* new_b = free_batches.front();
    free_batches.pop_front();
    decrypt_line(crypttxt, new_b->load_plaintxt(), new_b->plaintxt_size());
    cout << "PLAIN TXT: " << new_b->load_plaintxt() << endl;
    return new_b;
}