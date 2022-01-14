/*
 * Initialize and run a register server.
 */

#include "register_server.h"
#include <iostream>
#include <stdlib.h>


int main(int argc, char const *argv[]) {

    if (argc != 2) {
        // server ran incorrectly
        std::cout << "Usage: <EXE> [config_json]" << std::endl;
        return 1;
    }

    // initialize our server with the given port, and run it forever
    RegisterServer register_server(argv[1]);
    register_server.run();
    


    return 0;
}
