/*
 * Initialize and run a Server.
 */

#include "server.h"
#include <iostream>
#include <stdlib.h>
#include "gwas_u.h"
#include "enclave.h"

int main(int argc, char const *argv[]) {
    // let the OS assign a port unless specified otherwise
    int port = 0;

    if (argc != 3) {
        // server ran incorrectly
        std::cout << "Usage: <EXE> [port_number] [enclave_path]" << std::endl;
        return 1;
    } 
    else if (argc == 2){
        // set the port as the user specified
        port = atoi(argv[1]);
    }
    // initialize our server with the given port, and run it forever
    get_instance(port);

    boost::thread enclave_thread(start_enclave, argc, argv);
    enclave_thread.detach();

    get_instance().run();
    


    return 0;
}
