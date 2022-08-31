/*
 * Implementation of our instituion class.
 */


#include "institution.h"

Institution::Institution(std::string hostname, int port, int id, const int num_threads) 
        : hostname(hostname), port(port), requested_for_data(false), listener_running(false), 
          request_conn(-1), current_pos(0), all_data_received(false), id(id) {
    aes_encrypted_key_list.resize(num_threads);
    aes_encrypted_iv_list.resize(num_threads);

    encrypted_allele_data_size = 0;
    encrypted_allele_data_size_set = false;
}

Institution::~Institution() {

}

void Institution::add_block_batch(DataBlockBatch* block_batch) {
    std::lock_guard<std::mutex> raii(blocks_lock);
    blocks.push(block_batch);
}

int Institution::get_blocks_size() {
    return blocks.size();
}

int Institution::get_covariant_size() {
    return covariant_data.size();
}

int Institution::get_id() {
    return id;
}

void Institution::set_key_and_iv(std::string aes_key, std::string aes_iv, const int thread_id) {
    aes_encrypted_key_list[thread_id] = decoder.decode(aes_key);
    aes_encrypted_iv_list[thread_id] = decoder.decode(aes_iv);
}

void Institution::set_y_data(std::string& y_data) {
    y_val_data = decoder.decode(y_data);
    //std::memcpy(y_val_data, &decoder.decode(y_data)[0], 2528);
}

void Institution::set_covariant_data(const std::string& covariant_name, const std::string& data) {
    if (covariant_data.count(covariant_name)) {
        throw std::runtime_error("Duplicate covariant recieved.");
    }
    if (data.length() >= ENCLAVE_READ_BUFFER_SIZE) {
        throw std::runtime_error("Covariant too large for enclave: " + std::to_string(data.length()));
    }
    covariant_data[covariant_name] = decoder.decode(data);
}

std::string Institution::get_aes_key(const int thread_id) {
    return aes_encrypted_key_list[thread_id];
}

std::string Institution::get_aes_iv(const int thread_id) {
    return aes_encrypted_iv_list[thread_id];
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

int Institution::get_allele_data_size() {
    return encrypted_allele_data_size;
}

void Institution::transfer_eligible_blocks() {
    std::lock_guard<std::mutex> raii(blocks_lock);
    while(!blocks.empty()) {
        DataBlockBatch* batch = blocks.top();
        if(batch->pos != current_pos) {
            return;
        }
        blocks.pop();

        if (!encrypted_allele_data_size_set) {
            std::cout << "Set encrypted allele_data_size" << std::endl;
            encrypted_allele_data_size = batch->blocks_batch.front()->data.length();
            encrypted_allele_data_size_set = true;
        }
        for (DataBlock* parsed_block : batch->blocks_batch) {
            eligible_blocks.push(parsed_block);
        }
        // Clean up block batch!
        delete batch;
        current_pos++;
    }
}

DataBlock* Institution::get_top_block() {
    if (eligible_blocks.empty()) return nullptr;
    return eligible_blocks.front();
}

void Institution::pop_top_block() {
    eligible_blocks.pop();
}