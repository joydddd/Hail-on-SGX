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
#include <condition_variable>
#include <unordered_set>
#include <functional>
#include <fstream>
#include <boost/thread.hpp>

#include "institution.h"
#include "output.h"
#include "parser.h"
#include "json.hpp"
#include "aes-crypto.h"
#include "buffer_size.h"
#include "readerwriterqueue.h"

#define OUTPUT_FILE "gwasoutput/writeback"

enum EncMode { sgx, simulate, debug, NA };

class ComputeServer {
  private:
    nlohmann::json compute_config;

    int port;
    int num_threads;

    int global_id;
    int max_batch_lines;

    bool server_eof;

    EncMode enc_mode;
    EncAnalysis enc_analysis;
    ImputePolicy impute_policy;

    std::vector<bool> eof_read_list;

    std::unordered_set<std::string> expected_institutions;
    std::unordered_set<std::string> expected_covariants;
    std::vector<std::string> institution_list;
    std::vector<moodycamel::ReaderWriterQueue<std::string>> allele_queue_list;
    std::queue<std::string> output_queue;
    std::string covariant_list;
    std::string y_val_name;
    char* encrypted_aes_key;
    char* encrypted_aes_iv;

    uint8_t rsa_public_key[RSA_PUB_KEY_SIZE];
    AESCrypto encoder;

    std::unordered_map<std::string, Institution*> institutions;
  
    std::unordered_map<std::string, std::chrono::system_clock::time_point> enclave_clocks;
    std::unordered_map<std::string, unsigned long long> enclave_total_times;
    
    std::unordered_map<std::string, std::string> covariant_dtype;

    std::mutex expected_lock;

    std::mutex institutions_lock;

    std::mutex output_queue_lock;
    std::condition_variable output_queue_cv;

    // set up Server data structures
    void init(const std::string& config_file);
    
    // parses and calls the appropriate handler for an incoming client request
    bool handle_message(int connFD, const std::string& name, ComputeServerMessageType mtype, std::string& msg);

    // construct response header, encrypt response body, and send
    int send_msg(const std::string& name, const int mtype, const std::string& msg, int connFD=-1);
    int send_msg(const std::string& hostname, const int port, const int mtype, const std::string& msg, int connFD=-1);
    int send_msg_output(const std::string& msg, int connFD=-1);

    // start a thread that will handle a message and exit properly if it finds an error
    bool start_thread(int connFD, char* body_buffer);

    void check_in(const std::string& name);

    void data_requester();

    void data_listener(int connFD);

    void allele_matcher();

    void output_sender();

    void parse_header_compute_server_header(const std::string& header, std::string& msg,
                                            std::string& client_name, ComputeServerMessageType& mtype);

  public:

    ComputeServer(const std::string& config_file);

    ~ComputeServer();

    // create listening socket to handle requests on indefinitely
    void run();

    static ComputeServer* get_instance(const std::string& config_file="");

    static void start_timer(const std::string& func_name);

    static void stop_timer(const std::string& func_name);

    static void print_timings();

    static EncMode get_mode();

    static EncAnalysis get_analysis();

    static ImputePolicy get_impute_policy();

    static void finish_setup();

    static void set_max_batch_lines(unsigned int lines);

    static uint8_t* get_rsa_pub_key();
    
    static int get_num_threads();

    static int get_num_institutions();

    static std::string get_covariants();

    static std::string get_aes_key(const int institution_num, const int thread_id);

    static std::string get_aes_iv(const int institution_num, const int thread_id);

    static std::string get_num_patients(const int institution_num);

    static std::string get_y_data(const int institution_num);

    static std::string get_covariant_data(const int institution_num, const std::string& covariant_name);
    
    static int get_encrypted_allele_size(const int institution_num);

    static int get_allele_data(char* batch_data, const int thread_id);

    static void write_allele_data(char* output_data, const int buffer_size, const int thread_id);

    static void cleanup_output();
};

#endif /* _SERVER_H_ */
