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
#include <boost/thread.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "parser.h"
#include "socket_send.h"
#include "output.h"
#include "aes-crypto.h"

#ifndef _CLIENT_H_
#define _CLIENT_H_

#define REGISTER_MESSAGE "REGISTER"
#define BLOCK_SIZE 1

class Client {
  private:
    std::string clientname;
    std::string client_hostname;
    std::string server_hostname;
    int listen_port;
    int server_port;
    int blocks_sent;

    int server_num_threads;
    int num_patients;

    bool sender_running;
    bool sent_all_data;

    std::ifstream xval;

    std::vector<AESCrypto> aes_encryptor_list;

  public:
    Client(std::string clientname, std::string client_hostname, std::string server_hostname, int listen_port, int server_port);
    ~Client();

    // Set up Client data structures, establish connection with server
    void init();
    // Create listening socket to handle requests on indefinitely
    void run();

    // parses and calls the appropriate handler for an incoming client request
    void handle_message(int connFD, unsigned int size, ClientMessageType mtype, std::string& msg);

    // construct response header, encrypt response body, and send
    void send_msg(ComputeServerMessageType mtype, const std::string& msg, int connFD=-1);

    // start a thread that will handle a message and exit properly if it finds an error
    bool start_thread(int connFD);

    bool get_block(std::string& block); 

    void data_sender(int connFD);

    void send_tsv_file(std::string filename, ComputeServerMessageType mtype);

};

#endif /* _CLIENT_H_ */
