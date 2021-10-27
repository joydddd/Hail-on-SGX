/*
 * Implementation for our Parser class.
 */

#include "parser.h"
#include <assert.h>
#include <cctype>
#include <climits>
#include <iostream>
#include <string>
#include <stdexcept>
#include <utility>

using std::cout;
using std::endl;

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
    if (split_string != "") split_strings.push_back(split_string);
    return split_strings;
}

std::tuple<std::string, unsigned int, std::string> Parser::parse_header(std::string header) {
    // get username and size of message body
    // TODO: error checking
    auto words = split(header, ' ');
    if (words.size() != 3) throw std::runtime_error("Invalid header - not name, size, message type");
    unsigned int size = convert_to_num(words[1]);
    return std::make_tuple(words.front(), size, words.back());
}

int indexOfDifference(std::string cs1, std::string cs2) {
    if (cs1 == cs2) {
        return -1;
    }
    int i;
    for (i = 0; i < cs1.length() && i < cs2.length(); ++i) {
        if (cs1[i] != cs2[i]) {
            break;
        }
    }
    if (i < cs2.length() || i < cs1.length()) {
        return i;
    }
    return -1;
}

MessageType Parser::str_to_enum(std::string str_type) {
    MessageType mtype;
    if (str_type == "REGISTER") {
        mtype = REGISTER;
    }
    else if (str_type == "Y_VAL") {
        mtype = Y_VAL;
    }
    else if (str_type == "COVARIANT") {
        mtype = COVARIANT;
    }
    else if (str_type == "LOGISTIC") {
        mtype = LOGISTIC;
    }
    else if (str_type == "EOF_LOGISTIC") {
        mtype = EOF_LOGISTIC;
    }
    else {
        // TODO: insert error handling logic
        throw std::runtime_error("Failed to parse message type: " + str_type);
    }
    return mtype;
}

DataBlock* Parser::parse_body(std::string message_body, MessageType mtype) {
    if (mtype != LOGISTIC) return nullptr;
    std::vector<std::string> split_msg = Parser::split(message_body, ' ', 1);
    int counter = Parser::convert_to_num(split_msg.front());
    std::string encrypted_block = split_msg.back();
    struct DataBlock* block = new DataBlock;
    block->data = encrypted_block;
    block->pos = counter;
    return block;
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