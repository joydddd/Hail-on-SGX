/*
 * Header file for our parser class.
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <tuple>
#include <unordered_map>
#include <assert.h>
#include <cctype>
#include <climits>
#include <stdexcept>
#include <utility>

#include "aes-crypto.h"
#include "buffer_size.h"
#include "communication.h"

class Parser {
  private:
    static void two_bit_compress(uint8_t* input, uint8_t* compressed, unsigned int size);

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

    static int parse_nth_int(const std::string& str, const int n, const char delim='\t');

    // parse message body into the relevant arguments we need
    static void parse_data_body(std::vector<DataBlock*>& blocks, const std::string& message_body, AESCrypto& decoder);

    static int parse_compute_hash(const std::string& line, const int encryptor_list_size);

    // Returns the compute server we should send this allele to
    static void parse_allele_line(std::string& line, const int compute_server_hash, std::vector<uint8_t>& vals, std::vector<uint8_t>& compressed_vals, std::vector<std::vector<AESCrypto> >& aes_list_list);

    // split a given string based on the specified delimiter
    static void split(std::vector<std::string>& split_strings, const std::string& str, char delim=' ', int num_splits=-1);

    static void parse_connection_info(const std::string& str, ConnectionInfo& info, bool parse_num_threads=false);

    static void validate_path(const std::string& pathname, std::vector<std::string>& split_path);

    static unsigned int convert_to_num(const std::string& str);

    static void assert_non_empty(const std::string& str);
};

#endif /* _PARSER_H_ */
