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

struct BlockPointerBatchGT {
  inline bool operator()(const DataBlockBatch* a, const DataBlockBatch* b) const {
    return a->pos > b->pos;
  }
};

class Institution {
  private:
    std::mutex num_patients_lock;
    std::mutex blocks_lock;
    std::mutex covariant_data_lock;
    std::mutex y_val_data_lock;
    std::mutex aes_key_iv_lock;
    std::priority_queue<DataBlockBatch*, std::vector<DataBlockBatch* >, BlockPointerBatchGT > blocks;
    std::queue<DataBlock*> eligible_blocks;
    std::unordered_map<std::string, std::string> covariant_data;
    std::string y_val_data;
    std::string num_patients_encrypted;

    std::vector<std::string> aes_encrypted_key_list;
    std::vector<std::string> aes_encrypted_iv_list;

    int id;

  public:
    Institution(std::string hostname, int port, int id, const int num_threads);
    ~Institution();

    void set_key_and_iv(std::string aes_key, std::string aes_iv, const int thread_id);

    void add_block_batch(DataBlockBatch* block_batch);

    void transfer_eligible_blocks();

    void set_num_patients(const std::string& num_patients);

    void set_y_data(std::string& y_data);

    void set_covariant_data(const std::string& covariant_name, const std::string& data);

    std::string get_aes_key(const int thread_id);

    std::string get_aes_iv(const int thread_id);

    std::string get_num_patients();

    std::string get_y_data();

    std::string get_covariant_data(const std::string& covariant_name);

    int get_id();

    int get_blocks_size();

    int get_covariant_size();

    DataBlock* get_top_block();

    void pop_top_block();

    AESCrypto decoder;
    int port;
    int request_conn;

    int current_pos;
    bool requested_for_data;
    bool listener_running;
    bool all_data_received;

    std::string hostname;
    
};

#endif /* _institution_h_ */
