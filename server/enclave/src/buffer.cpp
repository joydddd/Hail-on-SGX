#ifdef ENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif
#include <chrono>
#include <thread>

#include "buffer.h"

using namespace std;

// TODO: impliment decryption in enclave
void enclave_decrypt(char crypt[ENCLAVE_READ_BUFFER_SIZE], string& plaintxt) {
    plaintxt = crypt;
}

void Batch::decrypt() {
    if (!strcmp(crypt, EndSeperator)) {
        e = true;
        r = true;
        f = true;
        return;
    }
    string plaintxt;
    enclave_decrypt(crypt, plaintxt);
    delete[] crypt;
    crypt = nullptr;
    stringstream pss(plaintxt);
    string line;
    while (getline(pss, line)) {
        if (line.empty()) continue;
        rows.push_back(line);
        stringstream lss(line);
        string loci_str;
        getline(lss, loci_str, '\t');
        locus.push_back(Loci(loci_str));
    }
    r = true;
}

void Batch::pop() {
    rows.pop_front();
    locus.pop_front();
    if (locus.empty()) f = true;
}

void Buffer::init() {
    working = vector<Batch*>(clients.size(), nullptr);
    loading = vector<Batch*>(clients.size(), nullptr);
    end = vector<bool>(clients.size(), false);
    for (int i = 0; i < clients.size(); i++) {
        client_map.insert(make_pair(clients[i], i));
    }
}

void Buffer::load_batch() {
    for (size_t i = 0; i < clients.size(); i++) {
        if (!loading[i] && !end[i]) {
            loading[i] = new Batch(LOG_t, clients[i]);
            bool* rt = new bool;
            getbatch(rt, clients[i].c_str(), type, loading[i]->load_addr());
            if (*rt) {
                loading[i]->decrypt();
                /* signal shifting */
            } else {
                delete loading[i];
                loading[i] = nullptr;
            }
            delete rt;
        }
    }
}

// call from computing
bool Buffer::shift_batch() {
    load_batch();  // for single threading purpose
    bool ready = true;
    for (size_t i = 0; i < clients.size(); i++) {
        // delete finished batches finshed computing
        if (end[i]) continue;
        if (working[i]) {
            if (!working[i]->finished()) continue;
            delete working[i];
            working[i] = nullptr;
        }

        // shift loading batch if ready
        if (!loading[i]) {
            ready = false;
            continue;
        }
        if (!loading[i]->ready()) {
            ready = false;
            continue;
        }
        working[i] = loading[i];
        loading[i] = nullptr;
        if (working[i]->end()) end[i] = true;

        /* signal loading here */
    }
    return ready;
}

Row* Buffer::get_nextrow(const Log_gwas& gwas) {
    /* shift until all the working batches are ready */
    while (!shift_batch()) {
        this_thread::sleep_for(chrono::milliseconds(BUFFER_UPDATE_INTERVAL));
    }

    /* check if all sequences has reached an end */
    bool finished = true;
    for (auto batch_end : end) {
        finished = batch_end && finished;
    }
    if (finished) return nullptr;

    /* find the smallest loci */
    Loci loci = Loci_MAX;
    for (auto batch : working) {
        if (batch->end()) continue;
        if (batch->toploci() < loci) loci = batch->toploci();
    }
    if (loci == Loci_MAX)
        throw CombineERROR("cannot find valid loci in buffer");
    // cerr << loci << endl;

    /* create row accordingly */
    Row* r = nullptr;
    for (auto batch : working) {
        Row* new_row = nullptr;
        switch (type) {
            case (XTX_t):
                new_row = new XTX_row();
                break;
            case (XTY_t):
                new_row = new XTY_row();
                break;
            case (SSE_t):
                new_row = new SSE_row();
                break;
            case (LOG_t):
                new_row = new Log_row(gwas);
                break;
        }
        if (!batch->end() && batch->toploci() == loci) {
            try {
                new_row->read(batch->toprow());
                batch->pop();
                if (r) {
                    r->combine(new_row);
                    delete new_row;
                } else {
                    r = new_row;
                }
            } catch (ReadtsvERROR& err) {
                batch->pop();
                cerr << "ERROR: invalid line: " << batch->toprow() << endl;
                cerr << err.msg << endl;
                r->append_invalid_elts(
                    client_col_num[client_map[batch->client()]]);
                delete new_row;
            }
        } else {
            if (!r)
                r = new_row;
            else
                delete new_row;
            r->append_invalid_elts(client_col_num[client_map[batch->client()]]);
        }
    }
    return r;
}

Buffer::~Buffer() {
    for (auto batch : loading) {
        if (batch) delete batch;
    }
    for (auto batch : working) {
        if (batch) delete batch;
    }
}

void OutputBuffer::write(const string& line) {
    if (size + line.size() >= ENCLAVE_OUTPUT_BUFFER_SIZE){
        writeback();
    }
    if (size + line.size() >= ENCLAVE_OUTPUT_BUFFER_SIZE){
        throw exportERROR("Enclave writeback buffer overflow");
    }
    strcpy(buffer + size, line.c_str());
    size += line.size();
}

void OutputBuffer::writeback() { 
    writebatch(type, buffer);
    size = 0;
}