#ifdef ENC_TEST
#include "enclave_old.h"
#else
#include "gwas_t.h"
#endif
#include "enc_gwas.h"
#include <chrono>
#include <thread>

using namespace std;

// TODO: impliment decryption in enclave
void enclave_decrypt(char crypt[ENCLAVE_READ_BUFFER_SIZE], string& plaintxt) {
    plaintxt = crypt;
}

void Batch::decrypt() {
    if (!strcmp(crypt, EndSperator)) {
        e = true;
        r = true;
        return;
    }
    string plaintxt;
    enclave_decrypt(crypt, plaintxt);
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
    if (locus.empty()) r = false;
}

void Buffer::init() {
    working = vector<Batch*>(clients.size(), nullptr);
    to_decrypt = vector<Batch*>(clients.size(), nullptr);
    for (int i = 0; i < clients.size(); i++) {
        client_map.insert(make_pair(clients[i], i));
    }
}

void Buffer::load_batch() {
    for (size_t i = 0; i < clients.size(); i++) {
        /* decrypt new batch if the working batch is empty */
        if (working[i]) {
            if (working[i]->ready()) continue;
            if (working[i]->end()) continue;
            delete working[i];
        }
        working[i] = to_decrypt[i];
        if (working[i]) working[i]->decrypt();

        /* get newbatch */
        to_decrypt[i] = new Batch(LOG_t, clients[i]);
        getbatch(clients[i].c_str(), type, to_decrypt[i]->crypt);
        if (!to_decrypt[i]->crypt) {
            delete to_decrypt[i];
            to_decrypt[i] = nullptr;
        }
    }
}

Row* Buffer::get_nextrow(const GWAS_logic& gwas) {
    /* update until all the working batches are ready */
    bool ready = false;
    while (true) {
        load_batch();
        ready = true;
        for (auto& batch : working) {
            if (!batch)
                ready = false;  // if batch is nullptr
            else if (!batch->ready())
                ready = false;
        }
        if (ready)
            break;
        else
            this_thread::sleep_for(
                chrono::milliseconds(BUFFER_UPDATE_INTERVAL));
    }
    bool end = true;
    for (auto batch : working) {
        if (!batch->end()) end = false;
    }
    if (end) return nullptr;

    /* find the smallest loci */
    Loci loci = Loci_MAX;
    for (auto batch : working) {
        if (batch->end()) continue;
        if (batch->toploci() < loci) loci = batch->toploci();
    }
    if (loci == Loci_MAX)
        throw CombineERROR("cannot find valid loci in buffer");

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
                new_row = new GWAS_row(gwas);
                break;
        }
        if (!batch->end() && batch->toploci() == loci) {
            try {
                new_row->read(batch->toprow());
            } catch (ReadtsvERROR& err){
                cerr << "ERROR: invalid line: " << batch->toprow() << endl;
                cerr << err.msg << endl;
                r->append_invalid_elts(client_col_num[client_map[batch->client()]]);
            }
            batch->pop();
            if (r) {
                r->combine(new_row);
                delete new_row;
            } else {
                r = new_row;
            }
        } else {
            if (!r) r = new_row;
            r->append_invalid_elts(client_col_num[client_map[batch->client()]]);
        }
    }
    return r;
}

bool OutputBuffer::extend(const string& line) {
    if (size + line.size() >= ENCLAVE_OUTPUT_BUFFER_SIZE) return false;
    strcpy(buffer + size, line.c_str());
    size += line.size();
    return true;
}

char* OutputBuffer::copy_to_host() {
    size = 0;
    return buffer;
}