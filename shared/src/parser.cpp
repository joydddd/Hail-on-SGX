/*
 * Implementation for our Parser class.
 */

#include "parser.h"
#include "hashing.h"

#include <iostream>

#include <unordered_map>
#include <string>

Parser::Parser(/* args */) {

}

Parser::~Parser() {

}

void Parser::split(std::vector<std::string>& split_strings, const std::string& str, char delim, int num_splits) {
    std::string split_string;
    int i = 0;
    for (char ch : str) {
        if (ch != delim || i == num_splits) {
            split_string += ch;
        } 
        else {
            ++i;
            split_strings.push_back(split_string);
            split_string.clear();
        }
    }
    if (split_string.length()) split_strings.push_back(split_string);
}

void Parser::parse_connection_info(const std::string& str, ConnectionInfo& info, bool parse_num_threads) {
    std::vector<std::string> parsed_info; 
    Parser::split(parsed_info, str, '\t');
    info.hostname = parsed_info[0];
    info.port = std::stoi(parsed_info[1]);

    if (parse_num_threads) {
        info.num_threads = std::stoi(parsed_info[2]);
    }
}

std::tuple<std::string, unsigned int, unsigned int> Parser::parse_header(const std::string& header) {
    // get username, size of message body, and message type
    std::vector<std::string> header_split;
    split(header_split, header, ' ');
    if (header_split.size() != 3) throw std::runtime_error("Invalid header - not name, size, message type");
    return std::make_tuple(header_split[0], convert_to_num(header_split[1]), convert_to_num(header_split[2]));
}

int Parser::parse_nth_int(const std::string& str, const int n, const char delim) {
    std::string parsed_int;
    int num_delims = 0;
    for (char c: str) {
        if (c == delim) {
            if (n == num_delims++) {
                break;
            }
            parsed_int = "";
        }
        parsed_int.push_back(c);
    }

    return std::stoi(parsed_int);
}

void Parser::parse_data_body(std::vector<DataBlock*>& blocks, const std::string& message_body, AESCrypto& decoder) {
    // TODO: optimize this... super inefficient currently

    std::vector<std::string> split_msg;
    Parser::split(split_msg, message_body, '\t');

    // Start idx at 1 to skip over the block id
    for (long unsigned int msg_idx = 1; msg_idx < split_msg.size(); msg_idx += 3) {
        DataBlock* block = new DataBlock;

        block->locus = split_msg[msg_idx] + "\t" + split_msg[msg_idx + 1];
        block->data = split_msg[msg_idx + 2];
        blocks.push_back(block);
    }
}

int Parser::parse_hash(const std::string& line, const int encryptor_list_size) {
    std::vector<std::string> line_split;
    Parser::split(line_split, line, '\t', 2);
    std::string locus_and_allele = line_split[0] + '\t' + line_split[1] + '\t';
    return hash_string(locus_and_allele, encryptor_list_size, false);
}

void Parser::parse_allele_line(std::string& line, 
                              std::vector<uint8_t>& vals, 
                              std::vector<uint8_t>& compressed_vals, 
                              std::vector<std::vector<AESCrypto> >& encryptor_list, 
                              const int compute_server_hash) {
    std::vector<std::string> line_split;
    Parser::split(line_split, line, '\t', 2);
    std::string locus_and_allele = line_split[0] + '\t' + line_split[1] + '\t';

    // Use the AES encryptor that corresponds to the appropriate thread on the server end
    std::vector<AESCrypto>& aes_list = encryptor_list[compute_server_hash];
    AESCrypto& encryptor = aes_list[hash_string(locus_and_allele, aes_list.size(), true)];

    std::string line_vals = line_split.back();
    int val_idx = 0;
    for (std::size_t line_idx = 0; line_idx < line_vals.length(); line_idx += 2) {
        switch(line_vals[line_idx]) {
            case '0':
                vals[val_idx++] = static_cast<char>(0);
                break;

            case '1':
                vals[val_idx++] = static_cast<char>(1);
                break;

            case '2':
                vals[val_idx++] = static_cast<char>(2);
                break;

            case 'N':
                vals[val_idx++] = static_cast<char>(3);
                line_idx++;
                break;

            default:
                throw std::runtime_error("Invalid alleles file!");
        }
    }
    two_bit_compress(&vals[0], &compressed_vals[0], vals.size());
    line = locus_and_allele + encryptor.encrypt_line((byte *)&compressed_vals[0], compressed_vals.size()) + "\n";
}

unsigned int Parser::convert_to_num(const std::string& str) {
    assert_non_empty(str);
    // check to make sure the number of digits isn't greater than the allowed number
    if (str.length() > std::to_string(UINT_MAX).length()) {
        throw std::runtime_error("Number is too big");
    }
    if (std::string::npos != str.find_first_not_of("0123456789")) {
        throw std::runtime_error("Number contains non-digits: " + str);
    }
    // convert the string to an unsigned long long to check its not too large
    // (even within the digit constraints, it could still be too large)
    if (strtoull(str.c_str(), nullptr, 10) > UINT_MAX) {
        throw std::runtime_error("Number is too big");
    }
    return stoul(str);
}

void Parser::assert_non_empty(const std::string& str) {
    if (!str.length()) {
        throw std::runtime_error("String is empty");
    }
}

void Parser::two_bit_compress(uint8_t* input, uint8_t* compressed, unsigned int size) {
    int two_bit_arr = 0;
    int two_bit_arr_count = 0;
    int compressed_idx = 0;
    for (unsigned int input_idx = 0; input_idx < size; ++input_idx) {
        two_bit_arr += input[input_idx] << (2 * two_bit_arr_count++);
        if (two_bit_arr_count == TWO_BIT_INT_ARR_SIZE) {
            compressed[compressed_idx++] = two_bit_arr;
            two_bit_arr = 0;
            two_bit_arr_count = 0;
        } 
    }
    // If our input is not a multiple of 4 we will have some trailing bits!
    if (two_bit_arr_count != 0) {
        compressed[compressed_idx] = two_bit_arr;
    }
}
