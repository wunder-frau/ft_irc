// main_test_join.cpp
#include <iostream>
#include <stdexcept>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

// Dummy send() implementation for testing purposes.
// The 'flags' parameter is unnamed to avoid unused parameter warnings.
ssize_t send(int socket, const void* buffer, size_t length, int /*flags*/) {
    std::cout << "[Socket " << socket << "] "
              << std::string(static_cast<const char*>(buffer), length);
    return length;
}

int main() {
    try {
        // Create a Server instance on port 6667 with a password.
        Server server(6667, "secret");

        // Create a dummy client with:
        // File descriptor: 1 and IP address "127.0.0.1"
        Client client(1, "127.0.0.1");
        client.setNickname("OldNick");
        client.setUsername("testuser");

        // Add the client to the server.
        server.addClient(client);

        // --- Test 1: Join a single new channel ---
        // Command: "JOIN #test"
        std::string joinCmd1 = "JOIN #test";
        std::cout << "\nProcessing join command: " << joinCmd1 << std::endl;
        server.handleJoin(1, joinCmd1);

        // --- Test 2: Join multiple channels with keys ---
        // Command: "JOIN #test,#random secret123,secret456"
        // This command tells the server to join two channels:
        //  - For channel "#test", the key "secret123" is provided.
        //  - For channel "#random", the key "secret456" is provided.
        std::string joinCmd2 = "JOIN #test,#random secret123,secret456";
        std::cout << "\nProcessing join command: " << joinCmd2 << std::endl;
        server.handleJoin(1, joinCmd2);

        // Optionally, add more tests or print additional server/channel status
        // here.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
