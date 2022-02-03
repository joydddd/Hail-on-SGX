/*
 * Initialize and run a Server.
 */

#include "compute_server.h"
#include <iostream>
#include <stdlib.h>

EncMode ComputeServer::enc_mode = NA;
int main(int argc, char const *argv[]) {

    if (argc == 2)
        ComputeServer::enc_mode = sgx;
    if (argc == 3 && !strcmp(argv[2], "--simulate"))
        ComputeServer::enc_mode = simulate;
    if (argc == 3 && !strcmp(argv[2], "--debug"))
        ComputeServer::enc_mode = debug;
    if (ComputeServer::enc_mode == NA){
        std::cout << "Usage: <EXE> [config_file] (--simlation)" << std::endl;
        return 1;
    }

    // initialize our server with the given port, and run it forever
    ComputeServer::get_instance(argv[1]);
    ComputeServer::get_instance().run();
    


    return 0;
}
