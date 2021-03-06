/*
 * Implementation for our server class.
 */

#include "register_server.h"
#include "socket_send.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdexcept>
#include <chrono>
#include <stdint.h>
#include <string>
#include "parser.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

std::mutex cout_lock;
std::mutex output_lock;

RegisterServer::RegisterServer(const std::string& config_file) {
    init(config_file);
}

RegisterServer::~RegisterServer() {

}

void RegisterServer::init(const std::string& config_file) {
    std::ifstream register_config_file(config_file);
    register_config_file >> register_config;
    port = register_config["register_server_bind_port"];
    compute_server_count = register_config["compute_server_count"];
    output_file.open(register_config["output_file_name"]);
}

void RegisterServer::run() {
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
	if (listen(sockfd, 300) < 0) {
        guarded_cout("listen: " + std::to_string(errno), cout_lock);
    }

    port = ntohs(addr.sin_port);
    guarded_cout("\n Running on port " + std::to_string(port), cout_lock);


    // (5) Serve incoming connections one by one forever (a lonely fate).
	while (true) {
        // accept connection from client
        int connFD = accept(sockfd, (struct sockaddr*) &addr, &addrSize);

        // spin up a new thread to handle this message
        boost::thread msg_thread(&RegisterServer::start_thread, this, connFD);
        msg_thread.detach();
    }
}

bool RegisterServer::start_thread(int connFD) {
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
        unsigned int body_size = std::stoi(header);

        char body_buffer[MAX_MESSAGE_SIZE];
        
        if (body_size != 0) {
            // read in encrypted body
            int rval = recv(connFD, body_buffer, body_size, MSG_WAITALL);
            if (rval == -1) {
                throw std::runtime_error("Error reading request body");
            }
        }
        std::string encrypted_body(body_buffer, body_size);
        std::vector<std::string> parsed_header;
        Parser::split(parsed_header, encrypted_body, ' ', 2);

        guarded_cout("ID/Client: " + parsed_header[0] + 
                     " Msg Type: " + parsed_header[1], cout_lock);
        //guarded_cout("\nEncrypted body:\n" + parsed_header[2], cout_lock);
        handle_message(connFD, static_cast<RegisterServerMessageType>(std::stoi(parsed_header[1])), parsed_header[2]);
    }
    catch (const std::runtime_error e)  {
        guarded_cout("Exception " + std::string(e.what()) + "\n", cout_lock);
        close(connFD);
        return false;
    }
    return true;
}

bool RegisterServer::handle_message(int connFD, RegisterServerMessageType mtype, std::string& msg) {
    std::string response;

    switch (mtype) {
        case COMPUTE_REGISTER:
        {
            ConnectionInfo compute_info;
            // Compare the max thread count of this machine with the others before it
            Parser::parse_connection_info(msg, compute_info);
            // Send the compute server its global ID
            
            send_msg(compute_info.hostname, compute_info.port, ComputeServerMessageType::GLOBAL_ID, std::to_string(compute_server_info.size()));

            std::lock_guard<std::mutex> raii(compute_lock);
            compute_server_info.push_back(msg);
            if (compute_server_info.size() == compute_server_count) {
                // Create message containing all compute server info
                for (std::string& info : compute_server_info) {
                    serialized_server_info.append(info + "\n");
                }
                // Remove trailing '\t'
                serialized_server_info.pop_back();

                // Send the compute server info to all waiting clients
                while(!institution_info_queue.empty()) {
                    ConnectionInfo institution_info = institution_info_queue.front();
                    institution_info_queue.pop();
                    send_msg(institution_info.hostname, institution_info.port, ClientMessageType::COMPUTE_INFO, serialized_server_info);
                }
            }
            break;
        }
        case CLIENT_REGISTER:
        {   
            // Parse body to get hostname and port
            ConnectionInfo institution_info;
            Parser::parse_connection_info(msg, institution_info);

            std::lock_guard<std::mutex> raii(compute_lock);
            // If we don't have all compute server info, add to waiting queue
            if (compute_server_info.size() != compute_server_count) {
                institution_info_queue.push(institution_info);
            } else {
                // If we have already recieved all compute server info, no need to wait!
                send_msg(institution_info.hostname, institution_info.port, ClientMessageType::COMPUTE_INFO, serialized_server_info);
            }
            break;
        }
        case OUTPUT:
        {
            //std::cout << msg << std::endl;// << " " << msg.front() << " " << msg.size() << std::endl;
            output_lock.lock();
            output_file << msg;
            output_file.flush();
            output_lock.unlock();
            //std::cout << msg << std::endl;
            break;
        }
        default:
            throw std::runtime_error("Not a valid response type");
    }
    
    close(connFD);
    return false;
}

int RegisterServer::send_msg(const std::string& hostname, const int port, int mtype, const std::string& msg, int connFD) {
    std::string message = "-1rs " + std::to_string(mtype) + " ";
    message = std::to_string(message.length() + msg.length()) + "\n" + message + msg;
    return send_message(hostname.c_str(), port, message.c_str(), message.length(), connFD);
}