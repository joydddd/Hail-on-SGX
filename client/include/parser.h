/*
 * Header file for our parser class.
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <vector>
#include <string>
#include <tuple>
#include <utility>
#include <unordered_map>

enum MessageType { 
  SUCCESS,
  DATA_REQUEST
};

struct DataBlock {
  std::string data;
  int pos;
};

class Parser {
  private:
    
  public:
    Parser(/* args */);
    ~Parser();
    // parse header, return a message type and encrypted body size pair
    static std::tuple<unsigned int, std::string> parse_header(std::string header);
    // decrypt the message body
    static std::string decrypt_body(std::string user, unsigned int size, 
                                    std::string encrypted_body, 
                                    std::unordered_map<std::string, std::string> &passwords);
    static std::string encrypt_response(std::string user, std::string response,
                                        std::unordered_map<std::string, std::string> &passwords);
    // returns enum for type of message we're dealing with
    static MessageType str_to_enum(std::string str_type);

    // parse message body into the relevant arguments we need
    static DataBlock* parse_body(std::string message_body, MessageType mtype);

    // split a given string based on the specified delimiter
    static std::vector<std::string> split(const std::string& s, char delim, int num_splits=-1);

    static void validate_path(const std::string& pathname, std::vector<std::string>& split_path);

    static void validate_arg_count(unsigned int size, MessageType mtype);

    static unsigned int convert_to_num(const std::string& str);

    static void assert_non_empty(const std::string& str);
};

#endif /* _PARSER_H_ */
