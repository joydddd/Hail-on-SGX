/*
 * Implementation for our server class.
 */

#include "server.h"
#include "socket_send.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include "parser.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "gwas_u.h"
#include "enclave.h"
#include "hashing.h"

using std::cout;
using std::cin;
using std::endl;

std::mutex cout_lock;

const int MIN_BLOCK_COUNT = 50;

Server::Server(int port_in) : port(port_in) {
    init();
}

Server::~Server() {

}

void Server::init() {
    // read in list institutions/clients involved in GWAS
    std::ifstream instituion_file("institutions.txt");
    std::string institution;
    while(getline(instituion_file, institution)) {
        expected_institutions.insert(institution);
        institution_list.push_back(institution);
    }

    // read in list of covariants
    std::ifstream covariant_file("covariants.txt");
    std::string covariant_line;
    while(getline(covariant_file, covariant_line)) {
        std::vector<std::string> covariant_and_dtype = Parser::split(covariant_line);
        std::string covariant = covariant_and_dtype.front();
        std::string dtype = covariant_and_dtype.back();
        covariant_dtype[covariant] = dtype;
        if (covariant != "1") {
            expected_covariants.insert(covariant);
        }
        covariant_list.append(covariant + " ");
    }

    // read in name of Y value
    std::ifstream y_file("y_val.txt");
    getline(y_file, y_val_name);

    num_threads = boost::thread::hardware_concurrency();

    // resize allele queue
    allele_queue_list.resize(num_threads);

    // Also start the enclave thread.
    boost::thread enclave_thread(start_enclave);
    enclave_thread.detach();
}

void Server::run() {
    // set up the server to do nothing when it receives a broken pipe error
    //signal(SIGPIPE, signal_handler);
    // create a master socket to listen for requests on
    // (1) Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // (2) Set the "reuse port" socket option
	int yesval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval));

    // (3) Create a sockaddr_in struct for the proper port and bind() to it.
	struct sockaddr_in addr;
    socklen_t addrSize = sizeof(addr);
    memset(&addr, 0, addrSize);

    // specify socket family (Internet)
	addr.sin_family = AF_INET;

	// specify socket hostname
	// The socket will be a server, so it will only be listening.
	// Let the OS map it to the correct address.
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // bind to our given port, or randomly get one if port = 0
	if (bind(sockfd, (struct sockaddr*) &addr, addrSize) < 0) {
        guarded_cout("bind failure: " + std::to_string(errno), cout_lock);
    } 

    // update our member variable to the port we just assigned
    if (getsockname(sockfd, (struct sockaddr*) &addr, &addrSize) < 0) {
        guarded_cout("getsockname failure: " + std::to_string(errno), cout_lock);
    }

    // (4) Begin listening for incoming connections.
	if (listen(sockfd, 30) < 0) {
        guarded_cout("listen: " + std::to_string(errno), cout_lock);
    }

    port = ntohs(addr.sin_port);
    guarded_cout("\n Running on port " + std::to_string(port), cout_lock);


    // (5) Serve incoming connections one by one forever (a lonely fate).
	while (true) {
        // accept connection from client
        int connFD = accept(sockfd, (struct sockaddr*) &addr, &addrSize);

        // spin up a new thread to handle this message
        boost::thread msg_thread(&Server::start_thread, this, connFD);
        msg_thread.detach();
    }
}

bool Server::start_thread(int connFD) {
    // if we catch any errors we will throw an error to catch and close the connection
    try {
        char header_buffer[128];
        // receive header, byte by byte until we hit deliminating char
        memset(header_buffer, 0, sizeof(header_buffer));

        int header_size = 0;
        bool found_delim = false;
        while (header_size < 128) {
            // Receive exactly one byte
            int rval = recv(connFD, header_buffer + header_size, 1, MSG_WAITALL);
            if (rval == -1) {
                throw std::runtime_error("Error reading stream message");
            }
            // Stop if we received a deliminating character
            if (header_buffer[header_size] == '\n') {
                found_delim = true;
                break;
            }
            header_size++;
        }
        if (!found_delim) {
            throw std::runtime_error("Didn't read in a null terminating char");
        }
        std::string header(header_buffer, header_size);
        // get the username and size of who sent this plaintext header
        auto parsed_header = Parser::parse_server_header(header);
        // guarded_cout("\nInstitution: " + std::get<0>(parsed_header) +
        // " Size: " + std::to_string(std::get<1>(parsed_header)) +
        // " Msg Type: " + std::to_string(std::get<2>(parsed_header)),
        // cout_lock);
        
        // read in encrypted body
        char body_buffer[8192];
        int rval = recv(connFD, body_buffer, std::get<1>(parsed_header), MSG_WAITALL);
        if (rval == -1) {
            throw std::runtime_error("Error reading request body");
        }
        std::string encrypted_body(body_buffer, std::get<1>(parsed_header));
        // guarded_cout("\nEncrypted body:\n" + encrypted_body, cout_lock);
        return handle_message(connFD, std::get<0>(parsed_header), std::get<1>(parsed_header), std::get<2>(parsed_header), encrypted_body);
    }
    catch (const std::runtime_error e)  {
        guarded_cout("Exception: " + std::string(e.what()), cout_lock);
        close(connFD);
        return false;
    }
    return true;
}

bool Server::handle_message(int connFD, const std::string& name, unsigned int size, ServerMessageType mtype,
                            std::string& msg) {
    DataBlock* block;
    if (institutions.count(name)) {
        block = Parser::parse_body(msg, mtype, institutions[name]->decoder);
    }

    std::string response;
    ClientMessageType response_mtype;

    switch (mtype) {
        case REGISTER:
        {
            if (institutions.count(name)) {
                throw std::runtime_error("User already registered");
            }
            std::vector<std::string> hostname_and_port = Parser::split(msg);
            for (int id = 0; id < institution_list.size(); ++id) {
                // Look for client id!
                if (institution_list[id] == name) {
                    institutions[name] = new Institution(hostname_and_port[0], 
                                                         Parser::convert_to_num(hostname_and_port[1]),
                                                         id,
                                                         num_threads);
                }
            }
            response_mtype = RSA_PUB_KEY;
            response = std::to_string(num_threads) + "\t" + reinterpret_cast<char *>(rsa_public_key);
            break;
        }
        case AES_KEY:
        {
            
            auto aes_info = Parser::split(msg, '\t');
            int thread_id = std::stoi(aes_info[2]);
            // We only want 1 "check in"
            if (thread_id == 0) {
                check_in(name);
            }
            institutions[name]->set_key_and_iv(
                                    aes_info[0], // encrypted key
                                    aes_info[1], // encrypted iv
                                    thread_id); // thread id      
            break;
        }
        case Y_VAL:
        {
            institutions[name]->set_y_data(msg);
            break;
        }
        case COVARIANT:
        {
            std::vector<std::string> name_data_split = Parser::split(msg, ' ', 1);
            std::string covariant_name = name_data_split.front();

            if (!expected_covariants.count(covariant_name)) {
                throw std::runtime_error("Unexpected covariant recieved.");
            }
            institutions[name]->set_covariant_data(covariant_name, name_data_split.back());

            // once we have all the covariants, request data
            if (expected_covariants.size() == institutions[name]->get_covariant_size()) {
                institutions[name]->request_conn = send_msg(name, DATA_REQUEST, std::to_string(MIN_BLOCK_COUNT), institutions[name]->request_conn);
                // start listener thread for data!
                boost::thread data_listener_thread(&Server::data_listener, this, institutions[name]->request_conn);
                data_listener_thread.detach();
            }
            break;
        }
        case EOF_DATA:
        {
            // intentionally missing break, we want to run the general data case after we do some clean up
            institutions[name]->all_data_received = true;
            if (block->data != EOFSeperator) {
                DataBlock* eof = new DataBlock;
                eof->data = EOFSeperator;
                eof->pos = block->pos + 1;
                institutions[name]->add_block(eof);
            }
        }
        case DATA:
        {
            institutions[name]->add_block(block);
            break;
        }
        default:
            throw std::runtime_error("Not a valid response type");
    }
    // now let's send that response
    if (response.length()) {
        send_msg(name, response_mtype, response);
    }
    if (mtype != DATA) {
        // cool, well handled!
        //guarded_cout("\nClosing connection", cout_lock);
        close(connFD);
        //guarded_cout("\n--------------", cout_lock);
        return false;
    }   
    return true;  
}

int Server::send_msg(const std::string& name, ClientMessageType mtype, const std::string& msg, int connFD) {
    std::string message = std::to_string(msg.length()) + " " + std::to_string(mtype) + '\n' + msg;
    return send_message(institutions[name]->hostname.c_str(), institutions[name]->port, message.c_str(), connFD);
}

void Server::check_in(std::string name) {
    std::lock_guard<std::mutex> raii(expected_lock);
    std::cout << name;
    // for (std::string inst : expected_institutions)
    if (!expected_institutions.count(name)) {
        throw std::runtime_error("Unexpected institution!\n");
    }
    expected_institutions.erase(name);
    // All clients checked in!
    if (!expected_institutions.size()) {
        // request y, cov, and data
        for (const auto& it : institutions) {
            send_msg(it.first, Y_AND_COV, covariant_list + y_val_name);
        }
        // Now that all institutions are registered, start up the allele matching thread.
        boost::thread msg_thread(&Server::allele_matcher, this);
        msg_thread.detach();
    }
}

void Server::data_listener(int connFD) {
    // We need a serial listener for this agreed upon connection!
    while(start_thread(connFD)) {}
}

void Server::allele_matcher() {
    while(true) {
    loop_start:
        // transfer all blocks to eligible queue, making sure they are in order
        for (const auto& it : institutions) {
            Institution* inst = it.second;
            inst->transfer_eligible_blocks();
        }
        // match as many alleles together as possible
        while(true) {
            // arbitary "high" string, all real loci should be smaller than this string.
            std::string min_locus = "~";
            for (const auto& it : institutions) {
                Institution* inst = it.second;
                DataBlock* block = inst->get_top_block();
                // check if the institution has data
                if (!block) {
                    // if we've already recieved an <EOF> notice and emptied the blocks pq, skip this block
                    if (inst->all_data_received && !inst->get_blocks_size()) {
                        continue;
                    }
                    // otherwise, we need to wait for all institutions to send their data!
                    else {
                        goto loop_start;
                    }
                }
                // test if block->locus < min_locus
                if (min_locus.compare(block->locus) > 0) {
                    min_locus = block->locus;
                }
            }

            // if we did not find a min locus, all data has been recieved and we have processed all of it.
            // enqueue EOF for all enclave threads then shut down the matcher, its work is done.
            if (min_locus == "~") {
                for(int thread_id = 0; thread_id < num_threads; ++thread_id) {
                    allele_queue_list[thread_id].enqueue(EOFSeperator);
                }
                return;
            }

            std::string allele_line = min_locus + "\t";
            std::string data;

            int locus_hash_thread = hash_string(allele_line, num_threads, true); 


            for (const auto& it : institutions) {
                Institution* inst = it.second;
                DataBlock* block = inst->get_top_block();
                // if this institution is <EOF>, skip it.
                if (!block) continue;

                if (block->locus == min_locus) {
                    inst->pop_top_block();
                    allele_line.append(std::to_string(inst->get_id()) + "\t");
                    data.append(block->data);
                    delete block;
                }
            }
            // replace last tab with a space so that we know where the data starts
            allele_line.pop_back();
            allele_line.push_back(' ');

            allele_line.append(data);
            
            // thread-safe enqueue
            allele_queue_list[locus_hash_thread].enqueue(allele_line);
        }
    }
}

Server& Server::get_instance(int port) {
    static Server instance(port);
    return instance;
}

uint8_t* Server::get_rsa_pub_key() {
    return get_instance().rsa_public_key;
}

int Server::get_num_threads() {
    return get_instance().num_threads;
}

int Server::get_num_institutions() {
    return get_instance().institution_list.size();
}

std::string Server::get_covariants() {
    std::string cov_list;
    std::vector<std::string> covariants = Parser::split(get_instance().covariant_list);
    for (std::string covariant : covariants) {
        cov_list.append(covariant + "\t");
    }
    return cov_list;
}

std::string Server::get_aes_key(const int institution_num, const int thread_id) {
    const std::string institution_name = get_instance().institution_list[institution_num];
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }

    return get_instance().institutions[institution_name]->get_aes_key(thread_id);
}

std::string Server::get_aes_iv(const int institution_num, const int thread_id) {
    const std::string institution_name = get_instance().institution_list[institution_num];
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }

    return get_instance().institutions[institution_name]->get_aes_iv(thread_id);
}

std::string Server::get_y_data(const int institution_num) {
    const std::string institution_name = get_instance().institution_list[institution_num];
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }
    std::string y_vals = get_instance().institutions[institution_name]->get_y_data();
    return y_vals;
}

std::string Server::get_covariant_data(const int institution_num, const std::string& covariant_name) {
    const std::string institution_name = get_instance().institution_list[institution_num];
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }
    std::string cov_vals = get_instance().institutions[institution_name]->get_covariant_data(covariant_name);
    return cov_vals;
}

int Server::get_encypted_allele_size(const int institution_num) {
    const std::string institution_name = get_instance().institution_list[institution_num];
    DataBlock* block = get_instance().institutions[institution_name]->get_top_block();
    if (!block) {
        return 0;
    }
    return block->data.length();
}

std::string Server::get_allele_data(int num_blocks, const int thread_id) {
    std::string allele_data;
    if (!get_instance().allele_queue_list[thread_id].try_dequeue(allele_data)) {
        return "";
    }
    return allele_data;
}
