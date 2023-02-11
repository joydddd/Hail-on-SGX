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
#include <thread>
#include "json.hpp"
#include "output.h"
#include "parser.h"

class RegisterServer {
  private:
    unsigned int port;
    unsigned int compute_server_count;
    unsigned int eof_messages_received;
    nlohmann::json register_config;

    std::vector<std::string> compute_server_info;
    std::string serialized_server_info;
    std::mutex compute_lock;

    std::vector<ConnectionInfo> institution_info_list;
    std::vector<ConnectionInfo> compute_info_list;

    std::vector<std::vector<std::string> > tmp_file_string_list;
    std::vector<std::mutex> tmp_file_mutex_list;

    std::ofstream output_file;

    std::string output_file_name;

    bool first;

    // set up data structures
    void init(const std::string& config_file);
    
    // parses and calls the appropriate handler for an incoming client request
    bool handle_message(int connFD, RegisterServerMessageType mtype, std::string& msg, std::string global_id);

    // send messages to the client
    int send_msg(const std::string& hostname, const int port, int mtype, const std::string& msg, int connFD=-1);

    // start a thread that will handle a message and exit properly if it finds an error
    bool start_thread(int connFD);

  public:

    RegisterServer(const std::string& config_file);

    ~RegisterServer();

    // create listening socket to handle requests on indefinitely
    void run();
};

#endif /* _REGISTER_SERVER_H_ */
