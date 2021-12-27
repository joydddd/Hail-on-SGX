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

std::vector<std::string> Parser::split(const std::string& s, char delim, int num_splits) {
    std::vector<std::string> split_strings;
    std::string split_string;
    int i = 0;
    for (char c : s) {
        if (c != delim || i == num_splits) {
            split_string += c;
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

std::tuple<unsigned int, ClientMessageType> Parser::parse_client_header(std::string header) {
    // get username and size of message body
    auto words = split(header, ' ');
    if (words.size() != 2) throw std::runtime_error("Invalid header - not size, message type");
    unsigned int size = convert_to_num(words.front());
    ClientMessageType mtype = static_cast<ClientMessageType>(convert_to_num(words.back()));
    return std::make_tuple(size, mtype);
}

std::tuple<std::string, unsigned int, ServerMessageType> Parser::parse_server_header(std::string header) {
    // get username and size of message body
    auto words = split(header, ' ');
    if (words.size() != 3) throw std::runtime_error("Invalid header - not name, size, message type");
    unsigned int size = convert_to_num(words[1]);
    ServerMessageType mtype = static_cast<ServerMessageType>(convert_to_num(words.back()));
    return std::make_tuple(words.front(), size, mtype);
}

DataBlock* Parser::parse_body(const std::string& message_body, ServerMessageType mtype, AESCrypto& decoder) {
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

std::string Parser::parse_allele_line(const std::string& line, std::string& vals, std::vector<AESCrypto>& encryptor_list, int num_patients) {
    std::vector<std::string> line_split = Parser::split(line, '\t', 2);
    std::string locus_and_allele = line_split[0] + '\t' + line_split[1] + '\t';
    // Use the AES encryptor that corresponds to the appropriate thread on the server end
    AESCrypto& encryptor = encryptor_list[hash_string(locus_and_allele, encryptor_list.size(), true)];

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
    return locus_and_allele + encryptor.encrypt_line((byte *)&vals[0], num_patients) + "\n";
}

unsigned int Parser::convert_to_num(const std::string& str) {
    assert_non_empty(str);
    // check to make sure the number of digits isn't greater than the allowed number
    if (str.length() > std::to_string(UINT_MAX).length()) {
        throw std::runtime_error("Number is too big");
    }
    if (std::string::npos != str.find_first_not_of("0123456789")) {
        throw std::runtime_error("Number contains non-digits");
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
