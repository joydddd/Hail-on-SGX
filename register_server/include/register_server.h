/*
 * Header file for our server class.
 */

#ifdef __APPLE__
#define MSG_NOSIGNAL 0x2000
#endif

#ifndef _REGISTER_SERVER_H_
#define _REGISTER_SERVER_H_

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <boost/thread.hpp>
#include "json.hpp"

#include "output.h"
#include "parser.h"

class RegisterServer {
  private:
    unsigned int port;
    unsigned int compute_server_count;
    nlohmann::json register_config;

    std::vector<std::string> compute_server_info;
    std::string serialized_server_info;
    std::mutex compute_lock;

    std::queue<SocketInfo> institution_info_queue;

    // set up data structures
    void init(const std::string& config_file);
    
    // parses and calls the appropriate handler for an incoming client request
    bool handle_message(int connFD, unsigned int size, RegisterServerMessageType mtype, std::string& msg);

    // send messages to the client
    int send_msg(const std::string& hostname, const int port, int mtype, const std::string& msg, int connFD=-1);

    // start a thread that will handle a message and exit properly if it finds an error
    bool start_thread(int connFD);

    void check_in_compute(std::string& msg);

    void check_in_institution();

  public:

    RegisterServer(const std::string& config_file);

    ~RegisterServer();

    // create listening socket to handle requests on indefinitely
    void run();
};

#endif /* _REGISTER_SERVER_H_ */
