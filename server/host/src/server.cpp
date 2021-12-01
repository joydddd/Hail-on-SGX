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
            covariant_list.append(covariant + " ");
        }
    }

    // read in name of Y value
    std::ifstream y_file("y_val.txt");
    getline(y_file, y_val_name);
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
            institutions[name] = new Institution(hostname_and_port[0], Parser::convert_to_num(hostname_and_port[1]));
            response_mtype = SUCCESS;
            response = "RSA KEY";
            break;
        }
        case AES_KEY:
        {
            check_in(name);
            auto key_and_iv = Parser::split(msg, '\t');
            institutions[name]->set_key_and_iv(
                                    key_and_iv.front(), 
                                    key_and_iv.back()
                                );
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
            institutions[name]->set_covariant_data(name_data_split.front(), name_data_split.back());
            break;
        }
        case EOF_DATA:
        {
            // intentionally missing break, we want to run the general data case after we do some clean up
            institutions[name]->all_data_recieved = true;
            if (block->data != "<EOF>") {
                DataBlock* eof = new DataBlock;
                eof->data = "<EOF>";
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
        guarded_cout("\nClosing connection", cout_lock);
        close(connFD);
        guarded_cout("\n--------------", cout_lock);
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
    if (!expected_institutions.count(name)) {
        throw std::runtime_error("Unexpected institution!\n");
    }
    expected_institutions.erase(name);
    if (!expected_institutions.size()) {
        // request y, cov, and data
        for (auto it : institutions) {
            Institution* inst = it.second;
            send_msg(it.first, Y_AND_COV, covariant_list + y_val_name);
            inst->request_conn = send_msg(it.first, DATA_REQUEST, std::to_string(MIN_BLOCK_COUNT), inst->request_conn);
            // start listener thread for data!
            boost::thread data_listener_thread(&Server::data_listener, this, inst->request_conn);
            data_listener_thread.detach();  
        }
        
    }
}

void Server::data_listener(int connFD) {
    // We need a serial listener for this agreed upon connection!
    while(start_thread(connFD)) {}
}

Server& Server::get_instance(int port) {
    static Server instance(port);
    return instance;
}

std::string Server::get_institutions() {
    std::string clientlist;
    for (std::string institution : get_instance().expected_institutions) {
        clientlist.append(institution + "\t");
    }
    return clientlist;
}

std::string Server::get_covariants() {
    std::string cov_list;
    std::vector<std::string> covariants = Parser::split(get_instance().covariant_list);
    for (std::string covariant : covariants) {
        cov_list.append(covariant + "\t");
    }
    return cov_list;
}

std::string Server::get_aes_key(const int institution_num) {
    const std::string institution_name = get_instance().institution_list[institution_num];
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }

    return get_instance().institutions[institution_name]->get_aes_key();
}

std::string Server::get_aes_iv(const int institution_num) {
    const std::string institution_name = get_instance().institution_list[institution_num];
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }

    return get_instance().institutions[institution_name]->get_aes_iv();
}

std::string Server::get_y_data(const std::string& institution_name) {
    //std::string y_list;
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }
    std::string y_vals = get_instance().institutions[institution_name]->get_y_data();
    // for (std::string y_val : y_vals) {
    //     y_list.append(y_val + "\t");
    // }
    return y_vals;
}

std::string Server::get_covariant_data(const std::string& institution_name, const std::string& covariant_name) {
    std::string cov_list;
    std::vector<std::string> cov_vals = Parser::split(get_instance().institutions[institution_name]->get_covariant_data(covariant_name));
    for (std::string cov_val : cov_vals) {
        cov_list.append(cov_val + "\t");
    }
    return cov_list;
}

std::string Server::get_x_data(const std::string& institution_name, int num_blocks) {
    return "";
    //return get_instance().institutions[institution_name]->get_blocks(num_blocks);
}
