/*
 * Header file for our client class.
 */

#include <string>
#include <iostream>
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

static const std::string REGISTER_MESSAGE = "REGISTER";

class Client {
  private:
    std::string clientname;
    std::string hostname;
    std::string server_hostname;
    int listen_port;
    int server_port;
    int blocks_sent;

  public:
    Client(std::string clientname, std::string hostname, std::string server_hostname, int listen_port, int server_port);
    ~Client();

    // Set up Client data structures, establish connection with server
    void init();
    // Create listening socket to handle requests on indefinitely
    void run();

    // parses and calls the appropriate handler for an incoming client request
    void handle_message(int connFD, unsigned int size, std::string msg_type, std::string& msg);

    // construct response header, encrypt response body, and send
    void send_msg(const std::string& msg_type, const std::string& msg);

    // start a thread that will handle a message and exit properly if it finds an error
    void start_thread(int connFD);

};

#endif /* _CLIENT_H_ */
