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
#include "helpers.h"


#ifndef _CLIENT_H_
#define _CLIENT_H_

#define REGISTER_MESSAGE "REGISTER"
#define BLOCK_SIZE 3

class Client {
  private:
    std::string clientname;
    std::string client_hostname;
    std::string server_hostname;
    int listen_port;
    int server_port;
    int blocks_sent;

    bool sender_running;

    std::ifstream xval;

  public:
    Client(std::string clientname, std::string client_hostname, std::string server_hostname, int listen_port, int server_port);
    ~Client();

    // Set up Client data structures, establish connection with server
    void init();
    // Create listening socket to handle requests on indefinitely
    void run();

    // parses and calls the appropriate handler for an incoming client request
    void handle_message(int connFD, unsigned int size, std::string msg_type, std::string& msg);

    // construct response header, encrypt response body, and send
    void send_msg(const std::string& msg_type, const std::string& msg, int connFD=-1);

    // start a thread that will handle a message and exit properly if it finds an error
    void start_thread(int connFD);

    bool get_block(std::string& block); 

    void data_sender(int connFD);

    void send_tsv_file(std::string filename, std::string mtype);

};

#endif /* _CLIENT_H_ */