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

void Institution::set_y_data(const std::string& y_data) {
    y_val_data = y_data;
}

void Institution::set_covariant_data(const std::string& covariant_name, const std::string& data) {
    covariant_data[covariant_name] = data;
}

std::string Institution::get_y_data() {
    return y_val_data;
}

std::string Institution::get_covariant_data(const std::string& covariant_name) {
    if (!covariant_data.count(covariant_name)) {
        return "";
    }
    return covariant_data[covariant_name];
}

std::string Institution::get_blocks(int num_blocks) {
    std::lock_guard<std::mutex> raii(blocks_lock);
    int count = 0;

    std::string res;
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