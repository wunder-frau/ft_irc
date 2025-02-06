#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "Client.hpp"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6667

int main() {
    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error: Cannot create socket\n";
        return 1;
    }

    // Connect to IRC server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr,
                sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Cannot connect to server\n";
        close(clientSocket);
        return 1;
    }

    std::cout << "Connected to " << SERVER_IP << ":" << SERVER_PORT << "\n";

    // Create Client instance
    Client ircClient(clientSocket);

    // Send basic IRC commands
    std::string nick = "TestUser";
    std::string user = "TestUser 0 * :Real Name";

    std::string nickCommand = "NICK " + nick + "\r\n";
    std::string userCommand = "USER " + user + "\r\n";

    send(clientSocket, nickCommand.c_str(), nickCommand.length(), 0);
    send(clientSocket, userCommand.c_str(), userCommand.length(), 0);

    // Handle user input
    ircClient.handleUserInput();

    // Close connection
    close(clientSocket);
    return 0;
}
