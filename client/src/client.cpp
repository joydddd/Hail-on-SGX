#include "client.h"
#include <boost/thread.hpp>
#include <chrono>
#include <thread>
#include <string>

std::mutex cout_lock;

Client::Client(const std::string& config_file) {
    init(config_file);
}

Client::~Client() {}

void Client::init(const std::string& config_file) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    std::ifstream client_config_file(config_file);
    client_config_file >> client_config;

    client_name = client_config["client_name"];
    client_hostname = get_hostname_str();

    listen_port = client_config["client_bind_port"];


    xval.open(client_config["allele_file"]);
    // remove first line from file
    std::string garbage;
    getline(xval, garbage);

    auto info = client_config["register_server_info"];
    send_msg(info["hostname"], info["port"], RegisterServerMessageType::CLIENT_REGISTER, client_hostname + "\t" + std::to_string(listen_port));

    num_patients = 0;
    work_distributed_count = 0;
    y_and_cov_count = 0;
    filled_count = 0;
    current_line_num = 0;
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
	if (listen(sockfd, 4096) < 0) {
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
                return false;
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
            return true;
        }
        unsigned int body_size;
        try {
            body_size = std::stoi(header);
        } catch(const std::invalid_argument &e) {
            std::cout << "Error in handling header: " << header << std::endl;
            throw e;
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

        guarded_cout("ID: " + parsed_header[0] + 
                     " Msg Type: " + parsed_header[1], cout_lock);
        // guarded_cout("\nEncrypted body:\n" + parsed_header[2], cout_lock);

        handle_message(connFD, std::stoi(parsed_header[0]), static_cast<ClientMessageType>(std::stoi(parsed_header[1])), parsed_header[2]);
    }
    catch (const std::runtime_error e)  {
        guarded_cout("Exception " + std::string(e.what()) + "\n", cout_lock);
        close(connFD);
        return false;
    }
    delete[] body_buffer;
    return true;
}

void Client::handle_message(int connFD, const unsigned int global_id, const ClientMessageType mtype, std::string& msg) {

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
            phenotypes_list.resize(num_compute_servers);
            allele_queue_list.resize(num_compute_servers);
            encryption_queue_list.resize(num_compute_servers);
            // Mutexes are not movable apparently :/
            std::vector<std::mutex> tmp(num_compute_servers);
            encryption_queue_lock_list.swap(tmp);

            for (int idx = 0; idx < num_compute_servers; ++idx) {
                allele_queue_list[idx] = new std::queue<std::string>();
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

            break;
        }
        case Y_AND_COV:
        {
            std::vector<std::string> covariants;
            Parser::split(covariants, msg);
            
            // the last covariant is actually our Y value, remove it from the list
            std::string y_val = covariants.back();
            covariants.pop_back();

            prepare_tsv_file(global_id, y_val, Y_VAL);
            // send the data in each TSV file over to the server
            for (std::string covariant : covariants) {
                // Ignore requests for "1", this is handled within the enclave
                if (covariant != "1") {
                    prepare_tsv_file(global_id, covariant, COVARIANT);
                }
            }
            
            if(++y_and_cov_count == aes_encryptor_list.size()) {
                fill_queue();
            }

            break;
        }
        case DATA_REQUEST:
        {   
            std::mutex useless_lock;
            std::unique_lock<std::mutex> useless_lock_wrapper(useless_lock);
            // Wait until all data is ready to go!
            while (filled_count != allele_queue_list.size()) {
                start_sender_cv.wait(useless_lock_wrapper);
            }
            auto start = std::chrono::high_resolution_clock::now();

            if (global_id == 0) {
                // Casting duration.count() to a string sucks, so RAII is difficult here
                cout_lock.lock();
                std::cout << "Sending first message: "  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
                cout_lock.unlock();
            }

            // First we should send all of the phenotype data
            std::vector<Phenotype>& phenotypes = phenotypes_list[global_id];
            for (const Phenotype& ptype : phenotypes) {
                std::thread([global_id, ptype, this](){
                    send_msg(global_id, ptype.mtype, ptype.message);
                }).detach();
            }

            ConnectionInfo info = compute_server_info[global_id];
            std::queue<std::string> *allele_queue = allele_queue_list[global_id];

            int blocks_sent = 0;
            std::string block;
            std::string lengths;

            std::string line;
            std::string line_length;
            int data_conn = -1;
            while (!allele_queue->empty()) {
                line = allele_queue->front();
                allele_queue->pop();
                line_length = "\t" + std::to_string(line.length());

                // 30 is magic number for extra padding
                int prospective_length = block.length() + line.length() + lengths.length() + line_length.length() + 30;
                if ((prospective_length > (1 << 16) - 1) && block.length()) {
                    // msg format: blocks sent \t lengths (tab delimited) \n (terminating char) blocks of data w no delimiters
                    std::string block_msg = std::to_string(blocks_sent++) + lengths + "\n" + block;
                    if (blocks_sent % 10 == 0) {
                        std::cout << "Blocks " << blocks_sent << std::endl;
                    }
                    data_conn = send_msg(info.hostname, info.port, DATA, block_msg, data_conn);

                    // Reset block
                    block.clear(); 
                    lengths.clear();
                }
                block += line;
                lengths += line_length;
            }
            // TODO: Re-evaluate why I compare it to 10? at some point this made a lot of sense to me, not it makes none
            // Send the leftover lines, 10 is an arbitary cut off. I assume most lines will be at least a few hundred characters
            // and we won't be sending more than 10^10 blocks
            if (block.length() > 10) {
                // msg format: blocks sent \t lengths (tab delimited) \n (terminating char) blocks of data w no delimiters
                std::string block_msg = std::to_string(blocks_sent++) + lengths + "\n" + block;
                send_msg(info.hostname, info.port, DATA, block_msg);
            }
            // If get_block failed we have reached the end of the file, send an EOF.
            send_msg(global_id, EOF_DATA, std::to_string(blocks_sent));


            if (global_id == 0) {
                std::cout << "Sending last message: "  << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;

                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "Data send time total: " << duration.count() << std::endl;
                xval.close();
            }
            break;
        }
        case END_CLIENT:
        {
            // Client has served its purpose! Exit the program
            exit(0);
            break;
        }
        default:
            throw std::runtime_error("Not a valid response type");
    }
    if (mtype != DATA_REQUEST) {
        close(connFD);
    }
}

void Client::send_msg(const unsigned int global_id, const unsigned int mtype, const std::string& msg, int connFD) {
    std::string message = client_name + " " + std::to_string(mtype) + " ";
    message = std::to_string(message.length() + msg.length()) + "\n" + message + msg;
    ConnectionInfo info = compute_server_info[global_id];
    send_message(info.hostname.c_str(), info.port, message.data(), message.length(), connFD);
}

int Client::send_msg(const std::string& hostname, unsigned int port, unsigned int mtype, const std::string& msg, int connFD) {
    std::string message = client_name + " " + std::to_string(mtype) + " ";
    message = std::to_string(message.length() + msg.length()) + "\n" + message + msg;
    return send_message(hostname.c_str(), port, message.data(), message.length(), connFD);
}

void Client::queue_helper(const int global_id, const int num_helpers) {
    std::string line;
    unsigned int line_num;
    auto start = std::chrono::high_resolution_clock::now();
    while(true) {
        xval_file_lock.lock();
        if (getline(xval, line)) {
            line_num = current_line_num++;
            xval_file_lock.unlock();
            // It's ok to have a benign data race here - each thread will do redundant writes of the same value.
            // I used to eliminate this redundancy by only having thread with id 0 do this write, but it is
            // theoretically (and practially...) possible for thread 0 to not get any lines causing
            // num_patients to never be written. This seems like the best solution!
            if (!num_patients) {
                // Subtract 2 for locus->alleles tab and alleles->first value tab
                std::vector<std::string> patients_split;
                Parser::split(patients_split, line, '\t');
                num_patients = patients_split.size() - 2;
            }

            EncryptionBlock *block = new EncryptionBlock();
            block->line_num = line_num;
            block->line = line;
            unsigned int compute_server_hash = Parser::parse_hash(block->line, aes_encryptor_list.size());
            encryption_queue_lock_list[compute_server_hash].lock(); 
            encryption_queue_list[compute_server_hash].push(block);
            encryption_queue_lock_list[compute_server_hash].unlock();
        } else {
            work_distributed_count++;
            queue_cv.notify_all();
            xval_file_lock.unlock();
            break;
        }
    }
    std::mutex useless_lock;
    std::unique_lock<std::mutex> useless_lock_wrapper(useless_lock);
    while(work_distributed_count != num_helpers) {
        queue_cv.wait(useless_lock_wrapper);
    }
    // We no longer need this thread if it is not in the encryption list range
    if (global_id >= encryption_queue_list.size()) {
        return;
    }

    std::vector<uint8_t> vals;
    std::vector<uint8_t> compressed_vals;
    vals.resize(num_patients);
    compressed_vals.resize((num_patients / TWO_BIT_INT_ARR_SIZE) + (num_patients % TWO_BIT_INT_ARR_SIZE ? 1 : 0));
    while (encryption_queue_list[global_id].size()) {
        EncryptionBlock *block = encryption_queue_list[global_id].top();
        encryption_queue_list[global_id].pop();
        line = block->line;
        delete block;

        Parser::parse_allele_line(line, 
                                  vals, 
                                  compressed_vals, 
                                  aes_encryptor_list, 
                                  global_id);
        allele_queue_list[global_id]->push(line);
    }
    if (++filled_count == allele_queue_list.size()) {
        // Make sure we read in at least one value
        int any = 0;
        for (int id = 0; id < compute_server_info.size(); ++id){
            any ^= allele_queue_list[id]->size();
        }
        if (!any) {
            throw std::runtime_error("Empty allele file provided");
        }
        start_sender_cv.notify_all();
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    if (global_id == 0) {
        // Using guarded_cout is hard here because converting duration.count() to a string sucks
        cout_lock.lock();
        std::cout << "Fill/encryption time total: " << duration.count() << std::endl;
        cout_lock.unlock();
    }
}

void Client::fill_queue() {
    const int num_helpers = std::max((unsigned int)boost::thread::hardware_concurrency(), (unsigned int)encryption_queue_list.size());
    for (int id = 0; id < num_helpers; ++id) {
        boost::thread helper_thread(&Client::queue_helper, this, id, num_helpers);
        helper_thread.detach();
    }
}

void Client::prepare_tsv_file(unsigned int global_id, const std::string& filename, ComputeServerMessageType mtype) {
    std::ifstream tsv_file("client_data/" + filename + ".tsv");
    std::string data;
    std::string line;

    std::vector<std::string> patient_and_data;
    int patient_count = -1;
    while(getline(tsv_file, line)) {
        patient_and_data.clear();
        Parser::split(patient_and_data, line, '\t');
        data.append(patient_and_data.back() + "\t");
        patient_count++;
    }
    data.pop_back();

    if (mtype == Y_VAL) {
        std::string patient_count_str = std::to_string(patient_count);
        patient_count_str = aes_encryptor_list[global_id].front().encrypt_line((byte *)&patient_count_str[0], patient_count_str.length());
        send_msg(global_id, ComputeServerMessageType::PATIENT_COUNT, patient_count_str);
    }

    // Some things are read by all threads (y values, covariants, etc.) and therefore 
    // should use the same AES key across all threads - we just thread id 0.
    data = aes_encryptor_list[global_id].front().encrypt_line((byte *)&data[0], data.length());
    if (mtype == COVARIANT) {
        data = filename + " " + data;
    }

    Phenotype ptype;
    ptype.message = data;
    ptype.mtype = mtype;
    phenotypes_list[global_id].push_back(ptype);
    tsv_file.close();
}