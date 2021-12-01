/*
 * Header file for our parser class.
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <vector>
#include <tuple>
#include <unordered_map>
#include <assert.h>
#include <cctype>
#include <climits>
#include <string>
#include <stdexcept>
#include <utility>

#include "aes-crypto.h"

enum ClientMessageType { 
  SUCCESS,
  Y_AND_COV,
  DATA_REQUEST
};

enum ServerMessageType { 
    REGISTER,
    AES_KEY,
    COVARIANT,
    Y_VAL,
    DATA,
    EOF_DATA
};

struct DataBlock {
  std::string locus;
  std::string data;
  int pos;
};

class Parser {
  private:
    
  public:
    Parser(/* args */);
    ~Parser();
    // parse header and return tuple containing relevant info
    static std::tuple<unsigned int, ClientMessageType> parse_client_header(std::string header);

    static std::tuple<std::string, unsigned int, ServerMessageType> parse_server_header(std::string header);

    // decrypt the message body
    static std::string decrypt_body(std::string user, unsigned int size, 
                                    std::string encrypted_body, 
                                    std::unordered_map<std::string, std::string> &passwords);
    static std::string encrypt_response(std::string user, std::string response,
                                        std::unordered_map<std::string, std::string> &passwords);

    // parse message body into the relevant arguments we need
    static DataBlock* parse_body(const std::string& message_body, ServerMessageType mtype, AESCrypto& decoder);

    static std::string parse_allele_line(const std::string& line, std::string& vals, AESCrypto& encryptor, int num_clients);

    // split a given string based on the specified delimiter
    static std::vector<std::string> split(const std::string& s, char delim=' ', int num_splits=-1);

    static void validate_path(const std::string& pathname, std::vector<std::string>& split_path);

    static unsigned int convert_to_num(const std::string& str);

    static void assert_non_empty(const std::string& str);
};

#endif /* _PARSER_H_ */
