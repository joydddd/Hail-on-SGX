/*
 * Implementation of our instituion class.
 */


#include "institution.h"

Institution::Institution(std::string hostname, int port) : hostname(hostname), port(port), 
                                                           requested_for_data(false), listener_running(false), 
                                                           request_conn(-1), current_block(0) {
}

Institution::~Institution() {

}

void Institution::add_block(DataBlock* block) {
    std::lock_guard<std::mutex> raii(blocks_lock);
    blocks.push(block);
}

int Institution::get_size() {
    return blocks.size();
}

std::string Institution::get_blocks(int num_blocks) {
    std::lock_guard<std::mutex> raii(blocks_lock);
    int count = 0;

    std::string res = "";
    while(count++ < num_blocks && !blocks.empty()) {
        DataBlock* block = blocks.top();
        if(block->pos != current_block) {
            break;
        }
        res.append(block->data);
        blocks.pop();
        delete block;
        current_block++;
    }
    return res;
}