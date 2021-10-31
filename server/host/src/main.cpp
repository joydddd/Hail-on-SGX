/*
 * Initialize and run a Server.
 */

#include "server.h"
#include <iostream>
#include <stdlib.h>
#include "gwas_u.h"
#include "enclave.h"

template <class T>
__attribute__((always_inline)) inline void DoNotOptimize(const T &value) {
  asm volatile("" : "+m"(const_cast<T &>(value)));
}

int main(int argc, char const *argv[]) {

    if (argc < 3) {
        // server ran incorrectly
        std::cout << "Usage: <EXE> [port_number] [enclave_path]" << std::endl;
        return 1;
    } 

    int port = atoi(argv[1]);

    // initialize our server with the given port, and run it forever
    Server::get_instance(port);
    DoNotOptimize(1);

    boost::thread enclave_thread(start_enclave, argc, argv);
    enclave_thread.detach();
    DoNotOptimize(2);

    Server::get_instance().run();
    


    return 0;
}
