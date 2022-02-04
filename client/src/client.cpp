#include "client.h"
#include <boost/thread.hpp>
#include <chrono>

std::mutex cout_lock;

Client::Client(const std::string& config_file) {
    init(config_file);
}

Client::~Client() {}

void Client::init(const std::string& config_file) {
    std::ifstream client_config_file(config_file);
    client_config_file >> client_config;

    client_name = client_config["client_name"];
    client_hostname = std::string(get_hostname_str());
    listen_port = client_config["client_bind_port"];


    xval.open(client_config["allele_file"]);
    // remove first line from file
    std::string garbage;
    getline(xval, garbage);

    auto info = client_config["register_server_info"];
    send_msg(info["hostname"], info["port"], RegisterServerMessageType::CLIENT_REGISTER, client_hostname + "\t" + std::to_string(listen_port));

    num_patients = 0;
    sender_running = false;
    sent_all_data = false;
    filled = false;
    y_and_cov_count = 0;
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
        guarded_cout("bind failure " + std::to_string(errno), cout_lock);
    } 

    // update our member variable to the port we just assigned
    if (getsockname(sockfd, (struct sockaddr*) &addr, &addrSize) < 0) {
        guarded_cout("getsockname failure: " + std::to_string(errno), cout_lock);
    }

    // (4) Begin listening for incoming connections.
	if (listen(sockfd, 300) < 0) {
        guarded_cout("listen: " + std::to_string(errno), cout_lock);
    }

    listen_port = ntohs(addr.sin_port);
    guarded_cout("\n Running on port " + std::to_string(listen_port), cout_lock);

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

        guarded_cout("ID: " + parsed_header[0] + 
                     " Msg Type: " + parsed_header[1], cout_lock);
        guarded_cout("\nEncrypted body:\n" + parsed_header[2], cout_lock);

        handle_message(connFD, std::stoi(parsed_header[0]), static_cast<ClientMessageType>(std::stoi(parsed_header[1])), parsed_header[2]);
    }
    catch (const std::runtime_error e)  {
        guarded_cout("Exception " + std::string(e.what()) + "\n", cout_lock);
        close(connFD);
        return false;
    }
    return true;
}

void Client::handle_message(int connFD, const unsigned int global_id, const ClientMessageType mtype, std::string& msg) {
    // if (sent_all_data) {
    //     return;
    // }

    std::string response;
    ComputeServerMessageType response_mtype;

    switch (mtype) {
        case COMPUTE_INFO:
        {
            std::vector<std::string> parsed_compute_info;
            Parser::split(parsed_compute_info, msg, '\n');
            // All received data should be formatted as:
            // [hostname]   [port]   [thread count]
            int aes_idx = 0;
            const int num_compute_servers = parsed_compute_info.size();
            aes_encryptor_list = std::vector<std::vector<AESCrypto> >(num_compute_servers);
            allele_queue_list.resize(num_compute_servers);
            for (int _ = 0; _ < num_compute_servers; ++_) {
                blocks_sent_list.push_back(0);
            }
            for (const std::string& compute_info : parsed_compute_info) {
                ConnectionInfo info;
                Parser::parse_connection_info(compute_info, info, true);
                compute_server_info.push_back(info);

                // Create AES keys for each thread of this compute server
                aes_encryptor_list[aes_idx++] = std::vector<AESCrypto>(info.num_threads);
            }

            for (int id = 0; id < compute_server_info.size(); ++id) {
                send_msg(id, REGISTER, client_hostname + "\t" + std::to_string(listen_port));
            }

            break;
        }
        case RSA_PUB_KEY:
        {
            // I wanted to use .resize() but the compiler cried about it, this is not ideal but acceptable.
            
            const std::string header = "-----BEGIN PUBLIC KEY-----";
            const std::string footer = "-----END PUBLIC KEY-----";

            size_t pos1 = msg.find(header);
            size_t pos2 = msg.find(footer, pos1+1);
            if (pos1 == std::string::npos || pos2 == std::string::npos) {
                throw std::runtime_error("PEM header/footer not found");
            }
            // Start position and length
            pos1 = pos1 + header.length();
            pos2 = pos2 - pos1;
            CryptoPP::StringSource pub_key_source(aes_encryptor_list[global_id].front().decode(msg.substr(pos1, pos2)), true);
            CryptoPP::RSA::PublicKey public_key;
            public_key.Load(pub_key_source);
            
            CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256> >::Encryptor rsa_encryptor(public_key);
            for (int thread_id = 0; thread_id < compute_server_info[global_id].num_threads; ++thread_id) {
                send_msg(global_id, AES_KEY, aes_encryptor_list[global_id][thread_id].get_key_and_iv(rsa_encryptor) + "\t" + std::to_string(thread_id));
            }
            guarded_cout("IMPLEMENT ATTESTATION!\n", cout_lock);
            break;
        }
        case Y_AND_COV:
        {
            std::vector<std::string> covariants;
            Parser::split(covariants, msg);
            
            // the last covariant is actually our Y value, remove it from the list
            std::string y_val = covariants.back();
            covariants.pop_back();

            send_tsv_file(global_id, y_val, Y_VAL);
            // send the data in each TSV file over to the server
            for (std::string covariant : covariants) {
                // Ignore requests for "1", this is handled within the enclave
                if (covariant != "1") {
                    send_tsv_file(global_id, covariant, COVARIANT);
                }
            }
            
            
            if(++y_and_cov_count == aes_encryptor_list.size()) {
                fill_queue();
                filled = true;
            }

            break;
        }
        case DATA_REQUEST:
        {   
            // Wait until all data is ready to go!
            while(!filled) {}
            auto start = std::chrono::high_resolution_clock::now();
            response_mtype = DATA;
            
            ConnectionInfo info = compute_server_info[global_id];
            std::queue<std::string>& allele_queue = allele_queue_list[global_id];
            int blocks_sent = 0;

            while(!allele_queue.empty()) {
                std::string block = std::to_string(blocks_sent++) + "\t";
                while(!allele_queue.empty()) {
                    std::string line = allele_queue.front();
                    
                    // If adding this line would cause us to exceed the max message size
                    // we are done with this batch of lines! Assume largest header possible is 30.
                    if ((block.length() + line.length() + 30) >= MAX_MESSAGE_SIZE) {
                        break;
                    }
                    block += line + "\t";
                    allele_queue.pop();
                }
                // Remove trailing '\t'
                block.pop_back();

                send_msg(info.hostname, info.port, response_mtype, block, connFD);
            }
            // If get_block failed we have reached the end of the file, send an EOF.
            std::string block = std::to_string(blocks_sent) + "\t" + EOFSeperator;
            response_mtype = EOF_DATA;
            send_msg(global_id, EOF_DATA, block, connFD);
            sent_all_data = true;
            
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "Data send time total: " << duration.count() << std::endl;
            break;
        }
        default:
            throw std::runtime_error("Not a valid response type");
    }
    if (mtype != DATA_REQUEST) {
        guarded_cout("\nClosing connection\n", cout_lock);
        close(connFD);
        guarded_cout("\n--------------\n", cout_lock);
    }
}

void Client::send_msg(const unsigned int global_id, const unsigned int mtype, const std::string& msg, int connFD) {
    std::string message = client_name + " " + std::to_string(mtype) + " ";
    message = std::to_string(message.length() + msg.length()) + "\n" + message + msg;
    ConnectionInfo info = compute_server_info[global_id];
    send_message(info.hostname.c_str(), info.port, message.c_str(), connFD);
}

void Client::send_msg(const std::string& hostname, unsigned int port, unsigned int mtype, const std::string& msg, int connFD) {
    std::string message = client_name + " " + std::to_string(mtype) + " ";
    message = std::to_string(message.length() + msg.length()) + "\n" + message + msg;
    //guarded_cout("MSG LENGTH: " + std::to_string(message.length()), cout_lock);
    send_message(hostname.c_str(), port, message.c_str(), connFD);
}

void Client::fill_queue() {
    std::string line;
    std::vector<uint8_t> vals;
    std::vector<uint8_t> compressed_vals;
    while(getline(xval, line)) {
        if (!num_patients) {
            // Subtract 2 for locus->alleles tab and alleles->first value tab
            std::vector<std::string> patients_split;
            Parser::split(patients_split, line, '\t');
            // This isn't very clean but I want to check the size of the encrypted/encoded line once
            num_patients = patients_split.size() - 2;
            vals.resize(num_patients);
            compressed_vals.resize((num_patients / TWO_BIT_INT_ARR_SIZE) 
                                    + (num_patients % TWO_BIT_INT_ARR_SIZE ? 1 : 0));
        }
        int compute_server_hash = Parser::parse_allele_line(line, vals, compressed_vals, aes_encryptor_list);
        allele_queue_list[compute_server_hash].push(line);
    }
}

void Client::data_sender(int connFD) {
    // We need a serial sender for this agreed upon connection!
    while(start_thread(connFD)) {}
}

void Client::send_tsv_file(unsigned int global_id, const std::string& filename, ComputeServerMessageType mtype) {
    std::ifstream tsv_file(filename + ".tsv");
    std::string data;
    // TODO: fix this code once Joy's enclave can handle a different format
    std::string line;
    // // read in first line garbage
    // getline(tsv_file, line);

    while(getline(tsv_file, line)) {
        //std::vector<std::string> patient_and_data = Parser::split(line, '\t');
        //data.append(patient_and_data.back() + " ");
        data.append(line + '\n');
    }
    // remove extra space at end of list
    data.pop_back();

    // Some things are read by all threads (y values, covariants, etc.) and therefore 
    // should use the same AES key across all threads - we just thread id 0.
    data = aes_encryptor_list[global_id].front().encrypt_line((byte *)&data[0], data.length());
    if (mtype == COVARIANT) {
        data = filename + " " + data;
    }
    send_msg(global_id, mtype, data);
}