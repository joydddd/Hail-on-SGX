#ifndef HASHING_H
#define HASHING_H

#include <string>

#define prime_a 533122189
#define prime_b 4156559549
#define prime_init_thread 59
#define prime_init_machine 71

unsigned int jump_hash(uint64_t key, uint32_t mod);

unsigned int hash_string(const std::string& input, uint32_t mod, bool thread_hash);

#endif