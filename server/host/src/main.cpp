/*
 * Initialize and run a Server.
 */

#include "server.h"
#include <iostream>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    // let the OS assign a port unless specified otherwise
    int port = 0;

    if (argc > 2) {
        // server ran incorrectly
        std::cout << "Usage: <EXE> [port_number]" << std::endl;
        return 1;
    } 
    else if (argc == 2){
        // set the port as the user specified
        port = atoi(argv[1]);
    }
    // initialize our server with the given port
    Server server(port);
    // run the server forever
    server.run();
    
    return 0;
}


