#include "batch.h"
#include <cstring>

Batch::Batch(size_t _row_size, Row_T row_type, ImputePolicy impute_policy, GWAS* _gwas, char *plaintxt_buffer)
    : row_size(_row_size), type(row_type) {
    switch (type) {
        case LOG_t:
            row = new Log_row(row_size, _gwas, impute_policy);
            break;
        case Lin_t:
            row = new Lin_row(row_size, _gwas, impute_policy);
            break;
        default:
            row = new Row(row_size, impute_policy);
            break;
    }
    plaintxt = plaintxt_buffer;
    batch_head = 0;
    st = Empty;
    txt_size = 0;
    out_tail = 0;
    row->reset();
}

void Batch::reset() {
    batch_head = 0;
    st = Empty;
    txt_size = 0;
    out_tail = 0;
}

Row* Batch::get_row(Buffer* buffer) {
    if (batch_head >= txt_size) {
        st = Finished;
        buffer->finish();
        return nullptr;
    }
    st = Working;
    row->reset();
    int res = row->read(plaintxt + batch_head);
    batch_head = batch_head + res;
#ifdef DEBUG
    // row->print();
#endif
    return row;
}

void Batch::write(const std::string& output) {
    strcpy(outtxt + out_tail, output.c_str());
    out_tail += output.size();
}

size_t Batch::get_out_tail() {
    return out_tail;
}