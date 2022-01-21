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

std::vector<std::string> Parser::split(const std::string& str, char delim, int num_splits) {
    std::vector<std::string> split_strings;
    std::string split_string;
    int i = 0;
    for (char ch : str) {
        if (ch != delim || i == num_splits) {
            split_string += ch;
        } 
        else {
            ++i;
            split_strings.push_back(split_string);
            split_string = "";
        }
    }
    if (split_string.length()) split_strings.push_back(split_string);
    return split_strings;
}

void Parser::parse_connection_info(const std::string& str, ConnectionInfo& info, bool parse_num_threads) {
    std::vector<std::string> parsed_info = Parser::split(str, '\t');
    info.hostname = parsed_info[0];
    info.port = std::stoi(parsed_info[1]);

    if (parse_num_threads) {
        info.num_threads = std::stoi(parsed_info[2]);
    }
}

std::tuple<std::string, unsigned int, unsigned int> Parser::parse_header(const std::string& header) {
    // get username, size of message body, and message type
    auto header_split = split(header, ' ');
    if (header_split.size() != 3) throw std::runtime_error("Invalid header - not name, size, message type");
    return std::make_tuple(header_split[0], convert_to_num(header_split[1]), convert_to_num(header_split[2]));
}

DataBlock* Parser::parse_body(const std::string& message_body, ComputeServerMessageType mtype, AESCrypto& decoder) {
    if (mtype != DATA && mtype != EOF_DATA) return nullptr;
    std::vector<std::string> split_line = Parser::split(message_body, '\t', 3);
    if (split_line.size() != 4) {
        split_line.push_back(EOFSeperator);
        split_line.push_back("");
        split_line.push_back("");
    }
    struct DataBlock* block = new DataBlock;
    block->pos = Parser::convert_to_num(split_line.front());
    block->locus = split_line[1] + '\t' + split_line[2];
    block->data = decoder.decode(split_line.back());
    
    return block;
}

int Parser::parse_allele_line(std::string& line, std::string& vals, std::vector<std::vector<AESCrypto> >& encryptor_list) {
    std::vector<std::string> line_split = Parser::split(line, '\t', 2);
    std::string locus_and_allele = line_split[0] + '\t' + line_split[1] + '\t';
    // Use the AES encryptor that corresponds to the appropriate thread on the server end
    int compute_server_hash = hash_string(locus_and_allele, encryptor_list.size(), false);

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
    line = locus_and_allele + encryptor.encrypt_line((byte *)&vals[0], vals.length()) + "\n";

    return compute_server_hash;
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
