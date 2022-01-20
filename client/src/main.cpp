#include <stdlib.h>
#include <string>
#include "client.h"


int main(int argc, const char **argv) {
	// Parse command line arguments
	if (argc != 2) {
		printf("Usage: ./client [config_file]\n");
		return 1;
	}

	Client client(argv[1]);
	client.run();

	return 0;
}