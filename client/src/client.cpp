#include "client.h"
#include <boost/thread.hpp>

using std::cout;
using std::cin;
using std::endl;

boost::mutex cout_lock;


Client::Client(std::string clientname, std::string client_hostname, std::string server_hostname, int listen_port, int server_port) 
    : clientname(clientname), client_hostname(client_hostname), server_hostname(server_hostname), 
    listen_port(listen_port), server_port(server_port), sender_running(false), xval("alleles.tsv") {
    init();
}

Client::~Client() {}

void Client::init() {
    // remove first line from file
    std::string garbage;
    getline(xval, garbage);

    send_msg("REGISTER", client_hostname + " " + std::to_string(listen_port));
    blocks_sent = 0;
}

void Client::run() {
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
    addr.sin_port = htons(listen_port);

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

    listen_port = ntohs(addr.sin_port);
    cout_lock.lock();
    cout << "\nRunning on port " << listen_port << endl;
    cout_lock.unlock();


    // (5) Serve incoming connections one by one forever (a lonely fate).
	while (true) {
        // accept connection from client
        int connFD = accept(sockfd, (struct sockaddr*) &addr, &addrSize);

        // spin up a new thread to handle this message
        boost::thread msg_thread(&Client::start_thread, this, connFD);
        msg_thread.detach();
    }
}

bool Client::start_thread(int connFD) {
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

        auto parsed_header = Parser::parse_header(header);
        cout_lock.lock();
        std::cout << "Size: " << std::get<0>(parsed_header) << " Msg Type: " << std::get<1>(parsed_header) << endl;
        cout_lock.unlock();
        
        char body_buffer[1024];
        if (std::get<0>(parsed_header) != 0) {
            // read in encrypted body
            int rval = recv(connFD, body_buffer, std::get<0>(parsed_header), MSG_WAITALL);
            if (rval == -1) {
                throw std::runtime_error("Error reading request body");
            }
        }
        std::string encrypted_body(body_buffer, std::get<0>(parsed_header));
        cout_lock.lock();
        cout << "Encrypted body:" << endl << encrypted_body << endl;
        cout_lock.unlock();
        handle_message(connFD, std::get<0>(parsed_header), std::get<1>(parsed_header), encrypted_body);
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

void Client::handle_message(int connFD, unsigned int size, std::string msg_type, std::string& msg) {
    MessageType mtype = Parser::str_to_enum(msg_type);

    std::string response;

    switch (mtype) {
        case SUCCESS:
        {
            std::cout << "IMPLEMENT ATTESTATION!\n";
            break;
        }
        case Y_AND_COV:
        {
            std::vector<std::string> covariants = Parser::split(msg);
            
            // the last covariant is actually our Y value, remove it from the list
            std::string y_val = covariants.back();
            covariants.pop_back();

            // send the data in each TSV file over to the server
            send_tsv_file(y_val, "Y_VAL");
            for (std::string covariant : covariants) {
                send_tsv_file(covariant, "COVARIANT");
            }
            break;
        }
        case DATA_REQUEST:
        {   
            msg_type = "LOGISTIC";
            std::string block;
            if (!sender_running) {
                sender_running = true;
                boost::thread data_sender_thread(&Client::data_sender, this, connFD);
            }
            for (int i = 0; i < std::stoi(msg); ++i) {
                if (!get_block(block)) {
                    msg_type = "EOF_LOGISTIC";
                    send_msg(msg_type, block, connFD);
                    break;
                }
                send_msg(msg_type, block, connFD);
            }
            break;
        }
        default:
            throw std::runtime_error("Not a valid response type");
    }
    // now let's send that response
    //send_response(connFD, name, response);
    // cool, well handled!
    if (mtype != DATA_REQUEST) {
        cout_lock.lock();
        cout << endl << "Closing connection" << endl;
        cout_lock.unlock();
        close(connFD);
        cout_lock.lock();
        cout << endl << "--------------" << endl;
        cout_lock.unlock();
    }
}

void Client::send_msg(const std::string& msg_type, const std::string& msg, int connFD) {
    std::string message = clientname + " " + std::to_string(msg.length()) + " " + msg_type +"\n" + msg;
    send_message(server_hostname.c_str(), server_port, message.c_str(), connFD);
}

bool Client::get_block(std::string& block) {
    std::string line;
    block = std::to_string(blocks_sent++) + " ";
    // TODO: see if we can read in BLOCK_SIZE lines in at a time
    for(int i = 0; i < BLOCK_SIZE; ++i) {
        if (!getline(xval, line)) {
            return false;
        }
        block += line + "\n";
    }
    return true;
}

void Client::data_sender(int connFD) {
    // We need a serial sender for this agreed upon connection!
    while(start_thread(connFD)) {}
}

void Client::send_tsv_file(std::string filename, std::string mtype) {
    std::ifstream tsv_file(filename + ".tsv");
    std::string data;
    if (mtype == "COVARIANT") {
        data.append(filename + " ");
    }
    
    std::string line;
    // read in first line garbage
    getline(tsv_file, line);

    while(getline(tsv_file, line)) {
        std::vector<std::string> patient_and_data = Parser::split(line, '\t');
        data.append(patient_and_data.back() + " ");
    }
    // remove extra space at end of list
    data.pop_back();

    send_msg(mtype, data);
}