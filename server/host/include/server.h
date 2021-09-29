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
#include <unordered_set>
#include <functional>
#include <boost/thread.hpp>
#include "institution.h"

class Server {
  private:
    int port;
    std::unordered_set<std::string> expected_institutions;
    std::unordered_map<std::string, Institution*> institutions;

    boost::mutex expected_lock;

  public:
    Server(int port_in);
    ~Server();

    // set up Server data structures
    void init();
    // create listening socket to handle requests on indefinitely
    void run();

    // parses and calls the appropriate handler for an incoming client request
    void handle_message(int connFD, const std::string& name, unsigned int size, std::string msg_type, std::string& msg);

    // construct response header, encrypt response body, and send
    void send_msg(const std::string& name, const std::string& msg_type, const std::string& msg);

    // start a thread that will handle a message and exit properly if it finds an error
    void start_thread(int connFD);

    void check_in(std::string name);

    void data_requester();
};

#endif /* _SERVER_H_ */
