/*
 * Header file for our institution class.
 */

#ifndef _INSTITUTION_H_
#define _INSTITUTION_H_

#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <boost/thread.hpp>
#include "parser.h"

struct BlockPointerGT {
  inline bool operator()(const DataBlock* a, const DataBlock* b) const {
    return a->pos > b->pos;
  }
};

class Institution {
  private:
    std::priority_queue<DataBlock*, std::vector<DataBlock*>, BlockPointerGT > blocks;
    boost::mutex blocks_lock;

  public:
    std::string hostname;
    int port;
    bool requested_for_data;

    Institution(std::string hostname, int port);
    ~Institution();

    void add_block(DataBlock* block);
    int get_size();
};

#endif /* _institution_h_ */
