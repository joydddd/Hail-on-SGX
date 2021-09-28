#ifndef ENC_BUFFER_H
#define ENC_BUFFER_H
#include "enc_gwas.h"

/* Buffer Management */

class Batch {
    /* meta data */
    string c;
    Row_T type;
    /* status */
    bool r = false;  // ready if has been decrypted and isn't empty
    bool e = false;  // true if this batch is the end of file.
    bool f = false;  // true if the batch has finished computing

    /* loading data */
    char *crypt;
    /* computing data */
    deque<string> rows;
    deque<Loci> locus;

   public:
    /* initialize */
    Batch(Row_T _type, string _client) : type(_type), c(_client) {
        crypt = new char[ENCLAVE_READ_BUFFER_SIZE];
    }
    ~Batch() {
        if (crypt) delete[] crypt;
    }

    /* return metadata/status */
    string client() { return c; }
    bool end() { return e; }    // is the last batch from this client
    bool ready() { return r; }  // ready for computing
    bool finished() { return f; }

    /* loading methods */
    char *load_addr() { return crypt; }
    void decrypt();

    /* computing methods */
    // need to check if the batch is empty before calling these methods
    Loci toploci() { return locus.front(); }
    void pop();
    string &toprow() { return rows.front(); }
};

class Buffer {
    /* define on init */
    map<string, int> client_map;
    vector<string> clients;
    vector<size_t> client_col_num;
    Row_T type;

    /* data */
    vector<Batch *> loading;
    vector<Batch *> working;

    /* status */
    vector<bool> end;  // track clients that have reaches their end

    /* mutex */
    // TODO

   public:
    /* initializing */
    Buffer(Row_T _type) : type(_type) {}
    void add_client(string _client, size_t _size = 0) {
        clients.push_back(_client);
        client_col_num.push_back(_size);
    }
    void init();  // init after adding all the clients

    /* loading methods */
    void load_batch();  // load batch if needed

    /* computing methods */
    bool shift_batch();  // shift batch from loading to computing. return true
                         // if all batches are available
    Row *get_nextrow(const Log_gwas &gwas = Log_gwas());
    // return nullptr if reaches end of all datasets

    // destructor
    ~Buffer();
};

class OutputBuffer {
    char buffer[ENCLAVE_OUTPUT_BUFFER_SIZE];
    Row_T type;
    size_t size = 0;

   public:
    OutputBuffer(Row_T _type) : type(_type), size(0) {}
    void write(const string &);  // return false if buffer is full
    void writeback();
    void print() const { printf("%s", buffer); }
};

#endif