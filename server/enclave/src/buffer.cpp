#include "buffer.h"
#include "string.h"

void decrypt(char* crypt, char* plaintxt, size_t* plaintxt_length){
    for(size_t i = 0; i<ENCLAVE_READ_BUFFER_SIZE; i++){
        plaintxt[i] = crypt[i];
    }
    *plaintxt_length = ENCLAVE_READ_BUFFER_SIZE;
}

void Batch::reset() { 
    head = 0;
    st = Empty;
    txt_size = 0;
}

Row* Batch::get_row(){
    if (head >= txt_size) {
        st = Finished;
        return nullptr;
    }
    st = Working;
    row->reset();
    head += row->read(plaintxt + head);
    return row;
}

Buffer::Buffer(size_t _row_size) : row_size(_row_size) {
    for (size_t i = 0; i < WORKING_THREAD_N; i++) {
        free_batches.push_back(new Batch(row_size));
    }
}

Buffer::~Buffer(){
    for(Batch* bat: free_batches){
        delete bat;
    }
}

void Buffer::finish(Batch* finishing_batch){
    finishing_batch->reset();
    free_batches.push_back(finishing_batch);
}

Batch* Buffer::launch(){
    bool rt;
    getbatch(&rt, crypttxt);
    if (!strcmp(crypttxt, EndSperator)) return nullptr;
    if (free_batches.empty()) return nullptr;
    Batch* new_b = free_batches.front();
    free_batches.pop_front();
    decrypt(crypttxt, new_b->load_plaintxt(), new_b->plaintxt_size());
}