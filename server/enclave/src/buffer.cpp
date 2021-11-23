#include "buffer.h"

#include "logistic_regression.h"
#include "string.h"


void decrypt(char* crypt, char* plaintxt, size_t* plaintxt_length) {
    for (size_t i = 0; i < ENCLAVE_READ_BUFFER_SIZE; i++) {
        plaintxt[i] = crypt[i];
    }
    *plaintxt_length = strlen(plaintxt);
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

void Batch::write(const string &output){
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

void Buffer::output(const char* out){
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
    if (!strcmp(crypttxt, EndSperator)) return nullptr;
    if (free_batches.empty()) return nullptr;
    Batch* new_b = free_batches.front();
    free_batches.pop_front();
    decrypt(crypttxt, new_b->load_plaintxt(), new_b->plaintxt_size());
    return new_b;
}