#include "hashing.h"

unsigned int jump_hash(uint64_t key, uint32_t mod) {
    int64_t b = -1, j=0;
    while (j < mod) {
        b =  j;
        key = (key * 2862933555777941757ULL) + 1;
        j = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
    }
    return b;
}

unsigned int hash_string(const std::string& input, uint32_t mod, bool thread_hash) {
    uint64_t hash = thread_hash ? prime_init_thread : prime_init_machine;
    bool start_hashing = false;
    for (const char c : input) {
        if (!start_hashing) {
            start_hashing = (c == ':');
            continue;
        }
        hash = (hash * prime_a) ^ (c * prime_b);
    }
    if (!start_hashing) {
        throw std::runtime_error("Colon never found in hashing process!\n");
    }
    return jump_hash(hash, mod);
}