// Handles an HTTP server connection
#include <iostream>
#include <exception>
#include <cstdlib>
#include <cctype>
#include "servers/sync-server.hpp"

bool isNumber(const char* s)
{
    if (s == nullptr || *s == '\0')
        return false;

    while (*s)
    {
        if (!std::isdigit(*s))
            return false;
        ++s;
    }
    return true;
}

int main(int argc, const char **argv)
{
    try
    {
        // Validate arguments
        if (argc < 3)
        {
            std::cerr << "Usage: " << argv[0] << " <host> <port>\n";
            return 1;
        }

        const char* host = argv[1];
        const char* portStr = argv[2];

        // Check if port is numeric
        if (!isNumber(portStr))
        {
            std::cerr << "Error: Port must be a numeric value. Given: " << portStr << "\n";
            return 2;
        }

        // (Optional) Convert to integer and validate range
        int port = std::atoi(portStr);
        if (port <= 0 || port > 65535)
        {
            std::cerr << "Error: Port must be in range 1–65535. Given: " << port << "\n";
            return 3;
        }

        mj::SyncServer server(host, portStr);
        server.run();
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return 4;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 5;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 6;
    }
    catch (...)
    {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 7;
    }

    return 0;
}
