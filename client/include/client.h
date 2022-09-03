/*
 * Header file for our client class.
 */

#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <boost/thread.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "readerwriterqueue.h"
#include "buffer_size.h"
#include "parser.h"
#include "socket_send.h"
#include "output.h"
#include "aes-crypto.h"
#include "json.hpp"
#include "phenotype.h"

#ifndef _CLIENT_H_
#define _CLIENT_H_

#define REGISTER_MESSAGE "REGISTER"
#define BLOCK_SIZE 1

class Client {
  private:
    nlohmann::json client_config;
    
    std::string client_name;
    std::string client_hostname;
    std::string server_hostname;
    int listen_port;
    int server_port;

    int num_patients;
    int num_lines_per_block;

    volatile bool work_distributed;

    std::ifstream xval;

    std::vector<std::vector<AESCrypto> > aes_encryptor_list;
    std::vector<std::vector<Phenotype> > phenotypes_list;
    std::vector<ConnectionInfo> compute_server_info;
    //std::vector<std::queue<std::string> > allele_queue_list;
    std::vector<moodycamel::ReaderWriterQueue<std::string>> allele_queue_list;
    std::vector<moodycamel::ReaderWriterQueue<std::string>> encryption_queue_list;
    std::vector<int> blocks_sent_list;
    std::atomic<int> y_and_cov_count;
    std::atomic<int> filled_count;
    std::condition_variable start_sender_cv;

  public:
    Client(const std::string& config_file);
    ~Client();

    // Set up Client data structures, establish connection with server
    void init(const std::string& config_file);
    // Create listening socket to handle requests on indefinitely
    void run();

    // parses and calls the appropriate handler for an incoming client request
    void handle_message(int connFD, const unsigned int global_id, const ClientMessageType mtype, std::string& msg);

    // construct response header, encrypt response body, and send
    void send_msg(const unsigned int global_id, const unsigned int mtype, const std::string& msg, int connFD=-1);
    void send_msg(const std::string& hostname, unsigned int port, unsigned int mtype, const std::string& msg, int connFD=-1);

    // start a thread that will handle a message and exit properly if it finds an error
    bool start_thread(int connFD);

    void queue_helper(const int global_id);

    void fill_queue();

    void data_sender(int connFD);

    void prepare_tsv_file(unsigned int global_id, const std::string& filename, ComputeServerMessageType mtype);

};

#endif /* _CLIENT_H_ */
