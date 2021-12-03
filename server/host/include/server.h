/*
 * Header file for our server class.
 */

#ifdef __APPLE__
#define MSG_NOSIGNAL 0x2000
#endif

#ifndef _SERVER_H_
#define _SERVER_H_

#include <unordered_map>
#include <string>
#include <queue>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <functional>
#include <boost/thread.hpp>

#include "institution.h"
#include "output.h"
#include "aes-crypto.h"
#include "buffer_size.h"
#include "readerwriterqueue.h"

class Server {
  private:
    int port;

    std::unordered_set<std::string> expected_institutions;
    std::unordered_set<std::string> expected_covariants;
    std::vector<std::string> institution_list;
    moodycamel::ReaderWriterQueue<std::string> allele_queue;
    std::string covariant_list;
    std::string y_val_name;
    char* encrypted_aes_key;
    char* encrypted_aes_iv;

    uint8_t rsa_public_key[RSA_PUB_KEY_SIZE];
    AESCrypto encoder;

    std::unordered_map<std::string, Institution*> institutions;
    
    std::unordered_map<std::string, std::string> covariant_dtype;

    std::mutex expected_lock;

    std::mutex institutions_lock;

    // set up Server data structures
    void init();
    
    // parses and calls the appropriate handler for an incoming client request
    bool handle_message(int connFD, const std::string& name, unsigned int size, ServerMessageType mtype, std::string& msg);

    // construct response header, encrypt response body, and send
    int send_msg(const std::string& name, ClientMessageType mtype, const std::string& msg, int connFD=-1);

    // start a thread that will handle a message and exit properly if it finds an error
    bool start_thread(int connFD);

    void check_in(std::string name);

    void data_requester();

    void data_listener(int connFD);

    void allele_matcher();

  public:

    Server(int port_in);

    ~Server();

    // create listening socket to handle requests on indefinitely
    void run();

    static Server& get_instance(int port=0);

    static uint8_t* get_rsa_pub_key();

    static std::string get_institutions();

    static std::string get_covariants();

    static std::string get_aes_key(const int institution_num);

    static std::string get_aes_iv(const int institution_num);

    static std::string get_y_data(const std::string& institution_name);

    static std::string get_covariant_data(const std::string& institution_name, const std::string& covariant_name);
    
    static int get_encypted_allele_size(const int institution_num);

    static std::string get_allele_data(int num_blocks);
};

#endif /* _SERVER_H_ */
