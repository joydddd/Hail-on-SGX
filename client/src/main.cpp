#include <stdlib.h>
#include <string>
#include "client.h"


int main(int argc, const char **argv) {
	// Parse command line arguments
	if (argc != 6) {
		printf("Usage: ./client client_name client_hostname listen_port server_hostname server_port\n");
		return 1;
	}

	Client client(argv[1], argv[2], argv[4], atoi(argv[3]), atoi(argv[5]));
	client.run();

	return 0;
}