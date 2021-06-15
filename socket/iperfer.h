#include <string>
struct SocketSetup {
    char mode;      // c for client, h for host
    char* node;     // e.g. "www.example.com" or IP
    char* service;  // e.g. "http" or port number
    int time;       // time out in seconds
    std::string filename; // file to send to recieve
};

#define BUFFER_SIZE 1024

int clientsetup(SocketSetup& ssetup);
void clientSend(int sock, std::string filename);
int serversetup(SocketSetup& ssetup);
void serverRec2File(int sock_acc, std::string filename);
void serverAcc(int socklisten, std::string filename);
