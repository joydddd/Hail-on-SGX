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
    std::unordered_map<std::string, std::string> covariant_data;
    std::string y_val_data;

  public:
    Institution(std::string hostname, int port);
    ~Institution();

    void add_block(DataBlock* block);

    void set_y_data(const std::string& y_data);

    void set_covariant_data(const std::string& covariant_name, const std::string& data);
  
    std::string get_y_data();

    std::string get_covariant_data(const std::string& covariant_name);

    int get_size();

    std::string get_blocks(int num_blocks);

    int port;
    int request_conn;

    int current_block;
    bool requested_for_data;
    bool listener_running;
    bool all_data_recieved;

    std::string hostname;
    
};

#endif /* _institution_h_ */
