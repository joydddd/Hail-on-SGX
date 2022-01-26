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
#include "buffer_size.h"

enum ClientMessageType {
  COMPUTE_INFO,
  RSA_PUB_KEY,
  Y_AND_COV,
  DATA_REQUEST
};

enum ComputeServerMessageType {
    GLOBAL_ID, 
    REGISTER,
    AES_KEY,
    COVARIANT,
    Y_VAL,
    DATA,
    EOF_DATA
};

enum RegisterServerMessageType {
  COMPUTE_REGISTER,
  CLIENT_REGISTER
};

struct ConnectionInfo {
  std::string hostname;
  unsigned int port;
  unsigned int num_threads; // only for compute server
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
    static std::tuple<std::string, unsigned int, unsigned int> parse_header(const std::string& header);

    // decrypt the message body
    static std::string decrypt_body(std::string user, unsigned int size, 
                                    std::string encrypted_body, 
                                    std::unordered_map<std::string, std::string> &passwords);
    static std::string encrypt_response(std::string user, std::string response,
                                        std::unordered_map<std::string, std::string> &passwords);

    // parse message body into the relevant arguments we need
    static DataBlock* parse_body(const std::string& message_body, ComputeServerMessageType mtype, AESCrypto& decoder);

    // Returns the compute server we should send this allele to
    static int parse_allele_line(std::string& line, std::string& vals, std::vector<std::vector<AESCrypto> >& encryptor_list);

    // split a given string based on the specified delimiter
    static void split(std::vector<std::string>& split_strings, const std::string& str, char delim=' ', int num_splits=-1);

    static void parse_connection_info(const std::string& str, ConnectionInfo& info, bool parse_num_threads=false);

    static void validate_path(const std::string& pathname, std::vector<std::string>& split_path);

    static unsigned int convert_to_num(const std::string& str);

    static void assert_non_empty(const std::string& str);
};

#endif /* _PARSER_H_ */
