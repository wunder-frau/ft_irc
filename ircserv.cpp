#include "Server.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    // optional fourth argument: -debug
    if (argc < 3 || argc > 4)
    {
        std::cerr << "Usage: ./ircserv <port> <password> [-debug]" << std::endl;
        return 1;
    }

    // Convert port argument to integer
    int port = std::atoi(argv[1]);
    std::string password = argv[2];
    bool debugMode = false;
    if (argc == 4 && std::string(argv[3]) == "-debug")
        debugMode = true;

    // Validate port range
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Error: Port must be between 1 and 65535." << std::endl;
        return 1;
    }

    try
    {
        // Create and run the server with debugMode set accordingly
        Server server(port, password, debugMode);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
