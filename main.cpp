
// Handles an HTTP server connection
#include <iostream>
#include "servers/sync-server.hpp"

int main(int argc, const char **argv)
{
     if (argc < 3)
    {
        std::cerr << "Invalid Host or port" << std::endl;
        return 1;
    }
   
    mj::SyncServer server(argv[1], argv[2]);
    server.run();
   
    return 0;
}
