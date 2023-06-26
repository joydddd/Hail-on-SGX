#include "batch.h"
#include <cstring>

#ifdef NON_OE
#include "enclave_glue.h"
#else
#include "gwas_t.h"
#endif

Batch::Batch(size_t _row_size, EncAnalysis analysis_type, ImputePolicy impute_policy, GWAS* _gwas, char *plaintxt_buffer, const std::vector<int>& sizes, int thread_id)
    : row_size(_row_size), type(analysis_type) {
    switch (analysis_type) {
        case EncAnalysis::logistic:
            row = new Log_row(row_size, sizes, _gwas, impute_policy);
            break;
        case EncAnalysis::linear:
            row = new Lin_row(row_size, sizes, _gwas, impute_policy, thread_id);
            break;
        case EncAnalysis::logistic_oblivious:
            row = new Oblivious_log_row(row_size, _gwas, impute_policy);
            break;
        case EncAnalysis::linear_oblivious:
            row = new Oblivious_lin_row(row_size, _gwas, impute_policy);
            break;
        default:
            throw std::runtime_error("No valid analysis type provided.");
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
        //start_timer("output()");
        buffer->finish();
        //stop_timer("output()");
        return nullptr;
    }
    //start_timer("parse_and_decrypt()");
    st = Working;
    //row->reset();
    int res = row->read(plaintxt + batch_head);
    batch_head = batch_head + res;
// #ifdef DEBUG
//     row->print();
// #endif
    return row;
}

void Batch::write(const std::string& output) {
    strcpy(outtxt + out_tail, output.c_str());
    out_tail += output.size();
}

size_t Batch::get_out_tail() {
    return out_tail;
}