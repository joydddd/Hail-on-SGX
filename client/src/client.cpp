#include "client.h"
#include <boost/thread.hpp>

std::mutex cout_lock;

Client::Client(std::string clientname, std::string client_hostname, std::string server_hostname, int listen_port, int server_port) 
    : clientname(clientname), client_hostname(client_hostname), server_hostname(server_hostname), 
      listen_port(listen_port), server_port(server_port), blocks_sent(0), num_clients(0), 
      sender_running(false), sent_all_data(false), xval("alleles.tsv") {
    init();
}

Client::~Client() {}

void Client::init() {
    // remove first line from file
    std::string garbage;
    getline(xval, garbage);

    send_msg(REGISTER, client_hostname + " " + std::to_string(listen_port));
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
	if (listen(sockfd, 30) < 0) {
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

        auto parsed_header = Parser::parse_client_header(header);
        guarded_cout("Size: " + std::to_string(std::get<0>(parsed_header)) + 
                     " Msg Type: " + std::to_string(std::get<1>(parsed_header)), cout_lock);
        
        char body_buffer[1024];
        if (std::get<0>(parsed_header) != 0) {
            // read in encrypted body
            int rval = recv(connFD, body_buffer, std::get<0>(parsed_header), MSG_WAITALL);
            if (rval == -1) {
                throw std::runtime_error("Error reading request body");
            }
        }
        std::string encrypted_body(body_buffer, std::get<0>(parsed_header));
        guarded_cout("\nEncrypted body:\n" + encrypted_body, cout_lock);

        handle_message(connFD, std::get<0>(parsed_header), std::get<1>(parsed_header), encrypted_body);
    }
    catch (const std::runtime_error e)  {
        guarded_cout("Exception " + std::string(e.what()) + "\n", cout_lock);
        close(connFD);
        return false;
    }
}

void Client::handle_message(int connFD, unsigned int size, ClientMessageType mtype, std::string& msg) {
    if (sent_all_data) {
        return;
    }

    std::string response;
    ServerMessageType response_mtype;

    switch (mtype) {
        case RSA_PUB_KEY:
        {
            
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

            CryptoPP::StringSource pub_key_source(aes_encryptor.decode(msg.substr(pos1, pos2)), true);
            CryptoPP::RSA::PublicKey public_key;
            public_key.Load(pub_key_source);
            
            CryptoPP::RSAES<CryptoPP::OAEP<CryptoPP::SHA256> >::Encryptor rsa_encryptor(public_key);

            send_msg(AES_KEY, aes_encryptor.get_key_and_iv(rsa_encryptor));
            guarded_cout("IMPLEMENT ATTESTATION!\n", cout_lock);
            break;
        }
        case Y_AND_COV:
        {
            std::vector<std::string> covariants = Parser::split(msg);
            
            // the last covariant is actually our Y value, remove it from the list
            std::string y_val = covariants.back();
            covariants.pop_back();

            // send the data in each TSV file over to the server
            send_tsv_file(y_val, Y_VAL);
            for (std::string covariant : covariants) {
                // Ignore requests for "1", this is handled within the enclave
                if (covariant != "1") {
                    send_tsv_file(covariant, COVARIANT);
                }
            }
            break;
        }
        case DATA_REQUEST:
        {   
            response_mtype = DATA;
            std::string block;
            while(get_block(block)) {
                send_msg(response_mtype, block, connFD);
            }
            // If get_block failed we have reached the end of the file, send an EOF.
            response_mtype = EOF_DATA;
            sent_all_data = true;
            send_msg(response_mtype, block, connFD);
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

void Client::send_msg(ServerMessageType mtype, const std::string& msg, int connFD) {
    std::string message = clientname + " " + std::to_string(msg.length()) + " " + std::to_string(mtype) +"\n" + msg;
    send_message(server_hostname.c_str(), server_port, message.c_str(), connFD);
}

bool Client::get_block(std::string& block) {
    std::string line;
    std::string vals;
    vals.resize(num_clients);

    block = std::to_string(blocks_sent++) + "\t";
    // TODO: see if we can read in BLOCK_SIZE lines in at a time
    for(int i = 0; i < BLOCK_SIZE; ++i) {
        if (!getline(xval, line)) {
            return false;
        }
        if (!num_clients) {
            // Subtract 2 for locus->alleles tab and alleles->first value tab
            num_clients = Parser::split(line, '\t').size() - 2;
            vals.resize(num_clients);
        }
        block.append(Parser::parse_allele_line(line, vals, aes_encryptor, num_clients)); 
    }
    return true;
}

void Client::data_sender(int connFD) {
    // We need a serial sender for this agreed upon connection!
    while(start_thread(connFD)) {}
}

void Client::send_tsv_file(std::string filename, ServerMessageType mtype) {
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
    data = aes_encryptor.encrypt_line((byte *)&data[0], data.length());
    if (mtype == COVARIANT) {
        data = filename + " " + data;
    }
    send_msg(mtype, data);
}