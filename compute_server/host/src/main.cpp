/*
 * Initialize and run a Server.
 */

#include "compute_server.h"
#include <iostream>
#include <stdlib.h>


int main(int argc, char const *argv[]) {

    if (argc < 3) {
        // server ran incorrectly
        std::cout << "Usage: <EXE> [port_number] [enclave_path]" << std::endl;
        return 1;
    } 

    int port = atoi(argv[1]);

    // initialize our server with the given port, and run it forever
    ComputeServer::get_instance(port);
    ComputeServer::get_instance().run();
    


    return 0;
}
