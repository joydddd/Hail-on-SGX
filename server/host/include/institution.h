/*
 * Header file for our institution class.
 */

#ifndef _INSTITUTION_H_
#define _INSTITUTION_H_

#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <mutex>
#include "parser.h"

struct BlockPointerGT {
  inline bool operator()(const DataBlock* a, const DataBlock* b) const {
    return a->pos > b->pos;
  }
};

class Institution {
  private:
    std::mutex blocks_lock;
    std::priority_queue<DataBlock*, std::vector<DataBlock* >, BlockPointerGT > blocks;
    std::queue<DataBlock*> eligible_blocks;
    std::unordered_map<std::string, std::string> covariant_data;
    std::string y_val_data;

    std::string aes_encrypted_key;
    std::string aes_encrypted_iv;

    int id;

  public:
    Institution(std::string hostname, int port, int id);
    ~Institution();

    void set_key_and_iv(std::string aes_key, std::string aes_iv);

    void add_block(DataBlock* block);

    void transfer_eligible_blocks();

    void set_y_data(std::string& y_data);

    void set_covariant_data(const std::string& covariant_name, const std::string& data);

    std::string get_aes_key();

    std::string get_aes_iv();
  
    std::string get_y_data();

    std::string get_covariant_data(const std::string& covariant_name);

    int get_id();

    int get_blocks_size();

    int get_covariant_size();

    DataBlock* get_top_block();

    void pop_top_block();

    std::string get_blocks(int num_blocks);

    int port;
    int request_conn;

    int current_block;
    bool requested_for_data;
    bool listener_running;
    bool all_data_received;

    AESCrypto decoder;

    std::string hostname;
    
};

#endif /* _institution_h_ */
