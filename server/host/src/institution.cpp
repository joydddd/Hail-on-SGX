/*
 * Implementation of our instituion class.
 */


#include "institution.h"

Institution::Institution(std::string hostname, int port) : hostname(hostname), port(port) {
    requested_for_data = false;
}

Institution::~Institution() {

}

void Institution::add_block(DataBlock* block) {
    blocks_lock.lock();
    blocks.push(block);
    std::cout << "Top block: " << blocks.top()->pos << std::endl;
    blocks_lock.unlock();
}

int Institution::get_size() {
    return blocks.size();
}