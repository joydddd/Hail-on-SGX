/*
 * Implementation of our instituion class.
 */


#include "institution.h"

Institution::Institution(std::string hostname, int port) : hostname(hostname), port(port), 
                                                           requested_for_data(false), listener_running(false), 
                                                           request_conn(-1), current_block(0), all_data_recieved(false) {
    // y_val_data = new char[10000];
}

Institution::~Institution() {

}

void Institution::add_block(DataBlock* block) {
    std::lock_guard<std::mutex> raii(blocks_lock);
    blocks.push(block);
}

int Institution::get_blocks_size() {
    return blocks.size();
}

int Institution::get_covariant_size() {
    return covariant_data.size();
}

void Institution::set_key_and_iv(std::string aes_key, std::string aes_iv) {
    aes_encrypted_key = decoder.decode(aes_key);
    aes_encrypted_iv = decoder.decode(aes_iv);
}

void Institution::set_y_data(std::string& y_data) {
    y_val_data = decoder.decode(y_data);
    //std::memcpy(y_val_data, &decoder.decode(y_data)[0], 2528);
}

void Institution::set_covariant_data(const std::string& covariant_name, const std::string& data) {
    if (covariant_data.count(covariant_name)) {
        throw std::runtime_error("Duplicate covariant recieved.");
    }
    covariant_data[covariant_name] = decoder.decode(data);
}

std::string Institution::get_aes_key() {
    return aes_encrypted_key;
}

std::string Institution::get_aes_iv() {
    return aes_encrypted_iv;
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
        res.append(block->locus + "\t" + block->data);
        blocks.pop();
        delete block;
        current_block++;
    }
    return res;
}