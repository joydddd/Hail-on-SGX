#include "batch.h"
#include <cstring>

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

Row* Batch::get_row(Buffer* buffer) {
    if (head >= txt_size) {
        st = Finished;
        buffer->finish();
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

size_t Batch::get_out_tail() {
    return out_tail;
}