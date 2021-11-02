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

class Server {
  private:
    int port;

    std::unordered_set<std::string> expected_institutions;
    std::string covariant_list;
    std::string y_val_name;

    std::unordered_map<std::string, Institution*> institutions;
    
    std::unordered_map<std::string, std::string> covariant_dtype;

    std::mutex expected_lock;

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

  public:
    Server(int port_in);

    ~Server();

    // create listening socket to handle requests on indefinitely
    void run();

    static Server& get_instance(int port=0);

    static std::string get_institutions();

    static std::string get_covariants();

    static std::string get_y_data(const std::string& institution_name);

    static std::string get_covariant_data(const std::string& institution_name, const std::string& covariant_name);

    static std::string get_x_data(const std::string& institution_name, int num_blocks);
};

#endif /* _SERVER_H_ */
