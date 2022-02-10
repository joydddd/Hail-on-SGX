/*
 * Initialize and run a Server.
 */

#include "compute_server.h"
#include <iostream>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: <EXE> [config_file] (optional: --simlation or --debug)" << std::endl;
        return 1;
    }

    // initialize our server and run it forever
    ComputeServer::get_instance(argv[1]);
    ComputeServer::get_instance().run();
    


    return 0;
}
