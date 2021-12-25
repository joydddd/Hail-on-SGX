#ifndef HASHING
#define HASHING

#include <string>

#define prime_a 533122189
#define prime_b 4156559549
#define prime_init_thread 59
#define prime_init_machine 71

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
    for (const char c : input) {
        hash = (hash * prime_a) ^ (c * prime_b);
    }
    return hash;
    return jump_hash(hash, mod);
}

#endif