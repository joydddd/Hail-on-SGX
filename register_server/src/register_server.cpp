/*
 * Implementation for our server class.
 */

#include "register_server.h"

std::mutex cout_lock;
std::condition_variable work_queue_condition;

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
    output_file_name = register_config["output_file_name"];
    output_file.open(output_file_name);
    
    tmp_file_string_list.resize(compute_server_count);

    std::vector<std::mutex> mux_tmp(compute_server_count);
    tmp_file_mutex_list.swap(mux_tmp);

    eof_messages_received = 0;
    first = true;
    shutdown = false;
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
	if (listen(sockfd, 4096) < 0) {
        guarded_cout("listen: " + std::to_string(errno), cout_lock);
    }

    port = ntohs(addr.sin_port);
    guarded_cout("\n Running on port " + std::to_string(port), cout_lock);


    // Make a thread pool to listen for connections!
    const uint32_t num_threads = std::thread::hardware_concurrency(); // Max # of threads the system supports
    for (uint32_t ii = 0; ii < num_threads; ++ii) {
        std::thread pool_thread = std::thread(&RegisterServer::start_thread, this);
        pool_thread.detach();
    }

    // (5) Serve incoming connections one by one forever (a lonely fate).
	while (true) {
        // accept connection from client
        int connFD = accept(sockfd, (struct sockaddr*) &addr, &addrSize);

        work_queue.enqueue(connFD);
        work_queue_condition.notify_all();

        // spin up a new thread to handle this message
        // boost::thread msg_thread(&RegisterServer::start_thread, this, connFD);
        // msg_thread.detach();
    }
}

void RegisterServer::start_thread() {
    start:

    int connFD;
    std::mutex useless_lock;
    std::unique_lock<std::mutex> useless_lock_wrapper(useless_lock);
    while (!work_queue.try_dequeue(connFD)) {
        if (shutdown) {
            work_queue_condition.notify_all();
            exit(0);
        }
        work_queue_condition.wait(useless_lock_wrapper);
    }
    char* body_buffer = new char[MAX_MESSAGE_SIZE]();
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
                throw std::runtime_error("Socket recv failed\n");
            } else if (rval == 0) {
                goto start;
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

        if (header.find("GET / HTTP/1.1") != std::string::npos) {
            std::cout << "Strange get request? Ignoring for now." << std::endl;
            delete[] body_buffer;
            goto start;
        }

        unsigned int body_size;
        try {
            body_size = std::stoi(header);
        } catch(const std::invalid_argument& e) {
            std::cout << "Failed to read in body size" << std::endl;
            std::cout << header << std::endl;
            goto start;
        }
        
        if (body_size != 0) {
            // read in encrypted body
            int rval = recv(connFD, body_buffer, body_size, MSG_WAITALL);
            if (rval == -1) {
                throw std::runtime_error("Error reading request body");
            }
        }
        std::string body(body_buffer, body_size);
        std::vector<std::string> parsed_header;
        Parser::split(parsed_header, body, ' ', 2);

        RegisterServerMessageType type = static_cast<RegisterServerMessageType>(std::stoi(parsed_header[1]));
        if (type != RegisterServerMessageType::OUTPUT) {
            guarded_cout("ID/Client: " + parsed_header[0] + 
                         " Msg Type: " + parsed_header[1], cout_lock);
        }
        //guarded_cout("\nEncrypted body:\n" + parsed_header[2], cout_lock);
        handle_message(connFD, type, parsed_header[2], parsed_header[0]);
    }
    catch (const std::runtime_error e)  {
        guarded_cout("Exception " + std::string(e.what()) + "\n", cout_lock);
        close(connFD);
        goto start;
    }
    delete[] body_buffer;
    goto start;
}

bool RegisterServer::handle_message(int connFD, RegisterServerMessageType mtype, std::string& msg, std::string global_id) {
    std::string response;

    switch (mtype) {
        case COMPUTE_REGISTER:
        {
            ConnectionInfo compute_info;
            // Compare the max thread count of this machine with the others before it
            Parser::parse_connection_info(msg, compute_info);
            // Send the compute server its global ID
            std::lock_guard<std::mutex> raii(compute_lock);
            send_msg(compute_info.hostname, compute_info.port, ComputeServerMessageType::GLOBAL_ID, std::to_string(compute_server_info.size()));
            std::cout << compute_info.hostname << " " << std::to_string(compute_server_info.size()) << std::endl;
            compute_info_list.push_back(compute_info);

            compute_server_info.push_back(msg);
            if (compute_server_info.size() == compute_server_count) {
                // Create message containing all compute server info
                for (std::string& info : compute_server_info) {
                    serialized_server_info.append(info + "\n");
                }
                // Remove trailing '\t'
                serialized_server_info.pop_back();

                // Send the compute server info to all waiting clients{
                for (ConnectionInfo institution_info : institution_info_list) {
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
            institution_info_list.push_back(institution_info);
            if (compute_server_info.size() == compute_server_count) {
                // If we have already received all compute server info, no need to wait!
                send_msg(institution_info.hostname, institution_info.port, ClientMessageType::COMPUTE_INFO, serialized_server_info);
            }
            break;
        }
        case OUTPUT:
        {
            int id = std::stoi(global_id);
            moodycamel::ConcurrentQueue<std::string> &tmp_file_string = tmp_file_string_list[id];
            tmp_file_string.enqueue(msg);
            break;
        }
        case EOF_OUTPUT:
        {
            if (strcmp(msg.c_str(), EOFSeperator) != 0) {
                int id = std::stoi(global_id);
                moodycamel::ConcurrentQueue<std::string> &tmp_file_string = tmp_file_string_list[id];
                tmp_file_string.enqueue(msg);
            }
            
            if (++eof_messages_received == compute_server_count) {
                std::cout << "Received last message: "  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                for (moodycamel::ConcurrentQueue<std::string>& tmp_file_string : tmp_file_string_list) {
                    std::string tmp;
                    while (tmp_file_string.try_dequeue(tmp)) {
                        std::vector<std::string> split;
                        Parser::split(split, tmp, '\n');
                        for (const std::string& tmp_split : split) {
                            sorted_file_queue.push(tmp_split);
                        }
                    }
                }

                while(!sorted_file_queue.empty()) {
                    output_file << sorted_file_queue.top() << std::endl;
                    sorted_file_queue.pop();
                }

                output_file.flush();

                std::vector<std::thread> msg_threads;
                // These aren't necesary for program correctness, but they help with iterative testing!
                for (ConnectionInfo institution_info : institution_info_list) {
                    msg_threads.push_back(std::thread([institution_info, this]() {
                        send_msg(institution_info.hostname, institution_info.port, ClientMessageType::END_CLIENT, "");
                    }));
                }
                for (ConnectionInfo compute_info : compute_info_list) {
                    msg_threads.push_back(std::thread([compute_info, this]() {
                        send_msg(compute_info.hostname, compute_info.port, ComputeServerMessageType::END_COMPUTE, "");
                    }));
                }
                for (std::thread &t : msg_threads) {
                    t.join();
                }

                // All files recieved, all shutdown messages sent, we can exit now
                shutdown = true;
                work_queue_condition.notify_all();
                exit(0);
            }
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