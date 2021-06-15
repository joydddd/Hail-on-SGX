#include "iperfer.h"

#include <getopt.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
using namespace std;

char getMode(int argc, char *argv[], SocketSetup &setup) {
    bool modeSpecified = false, hostSpecified = false, portSpecified = false;
    char mode = 'c';
    char *host = nullptr, *port = nullptr, *filename = nullptr;
    int time = 5;

    // These are used with getopt_long()
    opterr = true;  // Give us help with errors
    int choice;
    int option_index = 0;
    option long_options[] = {// TODO: Fill in two lines, for the "mode" ('m')
                             // and the "help" ('h') options.
                             {"server", no_argument, nullptr, 's'},
                             {"client", no_argument, nullptr, 'c'},
                             {"host", required_argument, nullptr, 'h'},
                             {"port", required_argument, nullptr, 'p'},
                             {"time", required_argument, nullptr, 't'},
                             {"file", required_argument, nullptr, 'f'},
                             {nullptr, 0, nullptr, '\0'}};

    // TODO: Fill in the double quotes, to match the mode and help options.
    while ((choice = getopt_long(argc, argv, "sch:p:t:f:", long_options,
                                 &option_index)) != -1) {
        switch (choice) {
            case 's':
                mode = 's';
                modeSpecified = true;
                break;
            case 'c':
                mode = 'c';
                modeSpecified = true;
                break;
            case 'h':
                host = optarg;
                hostSpecified = true;
                break;
            case 'p':
                port = optarg;
                portSpecified = true;
                break;
            case 't':
                time = stoi(optarg);
                break;
            case 'f':
                filename = optarg;
                break;
            default:
                cerr << "Error: invalid option on socket setup" << endl;
                exit(1);
        }  // switch
    }      // while

    if (!modeSpecified) {
        cerr << "Error: no mode specified" << endl;
        exit(1);
    }  // if
    if (!portSpecified) cerr << "Error: no port specified" << endl;
    if (mode == 'c' && !hostSpecified)
        cerr << "Error: no host specified" << endl;

    setup.mode = mode;
    setup.node = host;
    setup.service = port;
    setup.time = time;
    setup.filename = filename;
    return mode;
}  // getMode()

int clientsetup(SocketSetup &ssetup) {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    memset(&hints, 0, sizeof hints);  // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;      // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    // get ready to connect
    if ((status = getaddrinfo(ssetup.node, ssetup.service, &hints,
                              &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // servinfo now points to a linked list of 1 or more struct addrinfos
    int sock;
    sock = socket(servinfo->ai_family, servinfo->ai_socktype,
                  servinfo->ai_protocol);
    auto start_connect = time(0);
    while (connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        if (time(0)-start_connect > ssetup.time){
            fprintf(stderr,
                    "connect timeout on service %s, node %s: no response in %d "
                    "seconds\n",
                    ssetup.service, ssetup.node, ssetup.time);
            exit(1);
        }
    }
    freeaddrinfo(servinfo);
    return sock;
}

void clientSend(int sock, string filename) {
    ifstream f;
    f.open(filename, ios::in);
    char *buffer = new char[BUFFER_SIZE];
    size_t len, sent;
    size_t total_len = 0;
    if (!f.is_open()) {
        cerr << "Error: fail to open file " << filename << endl;
        return;
    }
    while (f.read(buffer, BUFFER_SIZE)) {
        len = strlen(buffer);
        sent = send(sock, buffer, len, 0);
        total_len += sent;
        while (sent != len) {
            sent += send(sock, buffer + sent, len - sent, 0);
            total_len += sent;
        }
    }
    len = f.gcount();
    f.read(buffer, len);
    sent = send(sock, buffer, len, 0);
    total_len += sent;
    while (sent != len) {
        sent += send(sock, buffer + sent, len - sent, 0);
        total_len += sent;
    }
    cout << "Sent " << filename << "(" << total_len << " bytes)" << endl;
    delete[] buffer;
    f.close();
}

int serversetup(SocketSetup &ssetup) {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    memset(&hints, 0, sizeof hints);  // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;      // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;      // fill in my IP for me

    if ((status = getaddrinfo(NULL, ssetup.service, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // servinfo now points to a linked list of 1 or more struct addrinfos

    int socklisten;
    socklisten = socket(servinfo->ai_family, servinfo->ai_socktype,
                        servinfo->ai_protocol);

    // ... do everything until you don't need servinfo anymore ....

    bind(socklisten, servinfo->ai_addr, servinfo->ai_addrlen);
    if (listen(socklisten, 5) == -1) {
        close(socklisten);
        fprintf(stderr, "listen error: on port %s", ssetup.service);
        exit(1);
    }

    freeaddrinfo(servinfo);
    cout << "start listening on port " << ssetup.service << " socket "
         << socklisten << endl;

    return socklisten;
}

void serverAcc(int socklisten, string filename){
    int i = 0;
    while (1) {
        socklen_t addr_size;
        struct sockaddr_storage their_addr;
        int sock_acc;
        addr_size = sizeof(their_addr);
        sock_acc =
            accept(socklisten, (struct sockaddr *)&their_addr, &addr_size);
        cerr << "Accepted on " << sock_acc << endl;
        if (sock_acc == -1) {
            cerr << "Error: fail to accept" << endl;
            continue;
        }
        serverRec2File(sock_acc, filename+to_string(i));
        ++i;
    }
}

void serverRec2File(int sock_acc, string filename) {
    ofstream f;
    f.open(filename);
    if (!f.is_open()) {
        cerr << "Error: fail to open file " << filename << endl;
    }
    int len, total_len=0;
    char *buffer = new char[BUFFER_SIZE];
    cout << "Recieve file " << filename; 
    while ((len = (int)recv(sock_acc, buffer, BUFFER_SIZE, 0)) != 0) {
        f.write(buffer, len);
        total_len += len;
    }
    cout << "(" << total_len << " bytes)" << endl;
    delete[] buffer;
    f.close();
}

int main(int argc, char *argv[]) {
    struct SocketSetup setupinfo;
    int sock;
    switch (getMode(argc, argv, setupinfo)) {
        case ('c'):
            sock = clientsetup(setupinfo);
            clientSend(sock, setupinfo.filename);
            break;
        case ('s'):
            sock = serversetup(setupinfo);
            serverAcc(sock, setupinfo.filename);
            break;
    }
}