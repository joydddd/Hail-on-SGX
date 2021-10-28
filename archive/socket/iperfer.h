#include <string>
struct SocketSetup {
    char mode = 'a';            // c for client, h for host
    char* node = nullptr;       // e.g. "www.example.com" or IP
    char* service = nullptr;    // e.g. "http" or port number
    int time = 5;               // time out for client in seconds, default to 5
    std::string filename;       // file to send to recieve
};

#define BUFFER_SIZE 1024

// return false if setup is not properly setup
bool CheckSetup(SocketSetup& setup);


int clientConnect(SocketSetup& ssetup); 
void clientSend(int sock, std::string filename);
int serverstart(SocketSetup& ssetup);
void serverRec2File(int sock_acc, std::string filename);
void serverAcc(int socklisten, std::string target_client, std::string filename);
