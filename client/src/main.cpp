#include <stdlib.h>
#include <string>
#include "helpers.h"
#include "client.h"


int main(int argc, const char **argv) {
	// Parse command line arguments
	if (argc != 6) {
		printf("Usage: ./client client_name hostname listen_port server_hostname server_port\n");
		return 1;
	}

	Client client(argv[1], argv[2], argv[4], atoi(argv[3]), atoi(argv[5]));
	client.run();
    

	// std::string message1 = "user1 1 SESSION\na";
	// std::string message2 = "user1 6 LOGISTIC\n1 msg1";
	// std::string message3 = "user1 6 LOGISTIC\n0 msg2";

	//printf("Sending message %s to %s:%d\n", message1.c_str(), server_name, server_port);
	//send_message(server_name, server_port, message1.c_str());
	//printf("Sending message %s to %s:%d\n", message2.c_str(), server_name, server_port);
	//send_message(server_name, server_port, message2.c_str());
	//printf("Sending message %s to %s:%d\n", message3.c_str(), server_name, server_port);
	//send_message(server_name, server_port, message3.c_str());

	return 0;
}