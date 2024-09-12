
// Handles an HTTP server connection
#include <iostream>
#include "servers/sync-server.hpp"

int main(int argc, const char **argv)
{
     if (argc < 3)
    {
        std::cerr << "give me address, port and doc root please." << std::endl;
        return 1;
    }
   
    mj::sync_server server;
    server.run();
   
    return 0;
}
