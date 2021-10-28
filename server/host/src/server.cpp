/*
 * Implementation for our server class.
 */

#include "server.h"
#include "helpers.h"
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

boost::mutex cout_lock;

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
    }

    // read in list of covariants
    std::ifstream covariant_file("covariants.txt");
    std::string covariant_line;
    while(getline(covariant_file, covariant_line)) {
        std::vector<std::string> covariant_and_dtype = Parser::split(covariant_line);
        std::string covariant = covariant_and_dtype.front();
        std::string dtype = covariant_and_dtype.back();
        covariant_dtype[covariant] = dtype;
        covariant_list.append(covariant + " ");
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
        cout_lock.lock();
        cout << "bind failure: " << errno << endl;
        cout_lock.unlock();
    } 

    // update our member variable to the port we just assigned
    if (getsockname(sockfd, (struct sockaddr*) &addr, &addrSize) < 0) {
        cout_lock.lock();
        cout << "getsockname failure: " << errno << endl;
        cout_lock.unlock();
    }

    // (4) Begin listening for incoming connections.
	if (listen(sockfd, 30) < 0) {
        cout_lock.lock();
        cout << "listen: " << errno << endl;
        cout_lock.unlock();
    }

    port = ntohs(addr.sin_port);
    cout_lock.lock();
    cout << "\nRunning on port " << port << endl;
    cout_lock.unlock();


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
    cout_lock.lock();
    cout << endl << "Accepted new connection" << endl;
    cout_lock.unlock();
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
        auto parsed_header = Parser::parse_header(header);
        cout_lock.lock();
        cout << "\nInstitution: " << std::get<0>(parsed_header) << " Size: " << std::get<1>(parsed_header) 
             << " Msg Type: " << std::get<2>(parsed_header) << endl;
        cout_lock.unlock();
        
        // read in encrypted body
        char body_buffer[8192];
        int rval = recv(connFD, body_buffer, std::get<1>(parsed_header), MSG_WAITALL);
        if (rval == -1) {
            throw std::runtime_error("Error reading request body");
        }
        std::string encrypted_body(body_buffer, std::get<1>(parsed_header));
        // cout_lock.lock();
        // cout << "Encrypted body:" << endl << encrypted_body << endl;
        // cout_lock.unlock();
        handle_message(connFD, std::get<0>(parsed_header), std::get<1>(parsed_header), std::get<2>(parsed_header), encrypted_body);
    }
    catch (const std::runtime_error e)  {
        cout_lock.lock();
        cout << "Exception: " << e.what() << endl;
        cout_lock.unlock();
        close(connFD);
        return false;
    }
    return true;
}

void Server::handle_message(int connFD, const std::string& name, unsigned int size, std::string msg_type,
                            std::string& msg) {
    MessageType mtype = Parser::str_to_enum(msg_type);
    DataBlock* block = Parser::parse_body(msg, mtype);

    std::string response;

    switch (mtype) {
        case REGISTER:
        {
            if (institutions.count(name)) {
                throw std::runtime_error("User already registered");
            }
            std::vector<std::string> hostname_and_port = Parser::split(msg);
            institutions[name] = new Institution(hostname_and_port[0], Parser::convert_to_num(hostname_and_port[1]));
            check_in(name);
            msg_type = "SUCCESS";
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
        case EOF_LOGISTIC:
        {
            // intentionally missing break, we want to run the logistic case after we do some clean up
            institutions[name]->all_data_recieved = true;
        }
        case LOGISTIC:
        {
            if (!institutions.count(name)) {
                throw std::runtime_error("No such user exists");
            }
            institutions[name]->add_block(block);
            institutions[name]->requested_for_data = false;
            break;
        }
        default:
            throw std::runtime_error("Not a valid response type");
    }
    // now let's send that response
    if (response.length()) {
        send_msg(name, msg_type, response);
    }
    if (mtype != LOGISTIC) {
        // cool, well handled!
        cout_lock.lock();
        cout << endl << "Closing connection" << endl;
        cout_lock.unlock();
        close(connFD);
        cout_lock.lock();
        cout << endl << "--------------" << endl;
        cout_lock.unlock();
    }     
}

int Server::send_msg(const std::string& name, const std::string& msg_type, const std::string& msg, int connFD) {
    std::string message = std::to_string(msg.length()) + " " + msg_type + '\n' + msg;
    return send_message(institutions[name]->hostname.c_str(), institutions[name]->port, message.c_str(), connFD);
}

void Server::check_in(std::string name) {
    std::lock_guard<std::mutex> raii(expected_lock);
    expected_institutions.erase(name);
    if (!expected_institutions.size()) {
        // start thread to request more data
        boost::thread requester_thread(&Server::data_requester, this);
        requester_thread.detach();
    }
}

void Server::data_requester() {
    while(true) {
        for (auto it : institutions) {
            Institution* inst = it.second;
            if (!inst->requested_for_data && inst->get_size() < MIN_BLOCK_COUNT && !inst->all_data_recieved) {
                inst->requested_for_data = true;
                inst->request_conn = send_msg(it.first, "DATA_REQUEST", std::to_string(MIN_BLOCK_COUNT), inst->request_conn);
                // Once we start asking for data, spin up a listener to reuse this connection
                if (!inst->listener_running) {
                    // Also ask for Y and covariant data
                    send_msg(it.first, "Y_AND_COV", covariant_list + y_val_name);

                    inst->listener_running = true;
                    boost::thread data_listener_thread(&Server::data_listener, this, inst->request_conn);
                    data_listener_thread.detach();  
                }
            }
        }
        boost::this_thread::sleep(boost::posix_time::seconds(1));
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

std::string Server::get_y_data(const std::string& institution_name) {
    std::string y_list;
    if (!get_instance().institutions.count(institution_name)) {
        return "";
    }
    std::vector<std::string> y_vals = Parser::split(get_instance().institutions[institution_name]->get_y_data());
    for (std::string y_val : y_vals) {
        y_list.append(y_val + "\t");
    }
    return y_list;
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
    return get_instance().institutions[institution_name]->get_blocks(num_blocks);
}
