#include "Server.hpp"
#include <cstdlib> // for std::atoi
#include <iostream>

int main(int argc, char *argv[])
{
    // Check for correct number of arguments
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    // Convert port argument to integer
    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    // Validate port range
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Error: Port must be between 1 and 65535." << std::endl;
        return 1;
    }

    try
    {
        // Create and run the server
        Server server(port, password);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
