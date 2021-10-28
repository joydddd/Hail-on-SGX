/*
 * Initialize and run a Server.
 */

#include "server.h"
#include <iostream>
#include <stdlib.h>
#include "gwas_u.h"
#include "enclave.h"

int main(int argc, char const *argv[]) {

    if (argc != 3) {
        // server ran incorrectly
        std::cout << "Usage: <EXE> [port_number] [enclave_path]" << std::endl;
        return 1;
    } 

    int port = atoi(argv[1]);

    // initialize our server with the given port, and run it forever
    volatile Server::get_instance(port);

    boost::thread enclave_thread(start_enclave, argc, argv);
    enclave_thread.detach();

    Server::get_instance().run();
    


    return 0;
}
