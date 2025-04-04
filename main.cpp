#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "Server.hpp"

#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024

int createServerSocket(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Allow socket descriptor to be reusable
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set socket to non-blocking
    if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Error setting socket to non-blocking" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Bind socket to a port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket to port " << port << std::endl;
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

void handleClientMessage(Server& server, int clientFd, const std::string& message, size_t* clientIndex) {
    std::cout << "Received from client " << clientFd << ": " << message << std::endl;

    // Check if client is registered, if not, try to register
    if (!server.isRegistered(clientFd)) {
        server.registerClient(clientFd, message, clientIndex);
        return;
    }

    // Parse the command
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    // Handle different commands
    if (command == "JOIN") {
        server.handleJoin(clientFd, message);
    } else if (command == "PART") {
        server.handlePart(clientFd, message);
    } else if (command == "INVITE") {
        server.handleInvite(clientFd, message);
    } else if (command == "KICK") {
        server.handleKick(clientFd, message);
    } else if (command == "TOPIC") {
        server.handleTopic(clientFd, message);
    } else if (command == "MODE") {
        server.handleMode(clientFd, message);
    } else if (command == "NICK") {
        server.nick(clientFd, message);
    } else if (command == "QUIT") {
        server.eraseClient(clientFd, clientIndex);
    } else {
        // Unknown command
        std::string nick = server.getClientObjByFd(clientFd)->getNick();
        server.sendError(clientFd, "421", nick, command + " :Unknown command");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return EXIT_FAILURE;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    // Create server instance
    Server server(port, password);

    // Create server socket
    int serverSocket = createServerSocket(port);
    std::cout << "IRC Server started on port " << port << std::endl;

    // Initialize pollfd array
    std::vector<pollfd> fds(1 + MAX_CLIENTS);
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN;
    
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;  // Initialize all client slots to -1
    }

    char buffer[BUFFER_SIZE];
    while (true) {
        // Wait for activity on one of the sockets
        int activity = poll(fds.data(), 1 + MAX_CLIENTS, -1);
        if (activity < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }

        // Check for new connections
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            
            if (clientSocket < 0) {
                std::cerr << "Error accepting connection" << std::endl;
            } else {
                std::cout << "New connection, socket fd: " << clientSocket 
                          << ", IP: " << inet_ntoa(clientAddr.sin_addr) 
                          << ", Port: " << ntohs(clientAddr.sin_port) << std::endl;
                
                // Set non-blocking
                if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) {
                    std::cerr << "Error setting client socket to non-blocking" << std::endl;
                }
#endif
                
                // Add to poll array
                bool added = false;
                for (int i = 1; i <= MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = clientSocket;
                        fds[i].events = POLLIN;
                        added = true;
                        
                        // Create and add client to server
                        Client newClient(clientSocket, inet_ntoa(clientAddr.sin_addr));
                        server.addClient(newClient);
                        break;
                    }
                }
                
                if (!added) {
                    std::cerr << "Too many clients, connection rejected" << std::endl;
                    close(clientSocket);
                }
            }
        }

        // Check existing connections for data
        for (int i = 1; i <= MAX_CLIENTS; i++) {
            if (fds[i].fd != -1 && (fds[i].revents & POLLIN)) {
                int clientFd = fds[i].fd;
                memset(buffer, 0, BUFFER_SIZE);
                
                int bytesRead = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
                if (bytesRead <= 0) {
                    // Connection closed or error
                    std::cout << "Client disconnected, socket fd: " << clientFd << std::endl;
                    close(clientFd);
                    fds[i].fd = -1;
                    
                    // Remove client from server
                    size_t clientIndex = i;
                    server.eraseClient(clientFd, &clientIndex);
                } else {
                    // Process message
                    buffer[bytesRead] = '\0';
                    std::string message(buffer);
                    
                    // Remove trailing newlines and carriage returns
                    while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) {
                        message.pop_back();
                    }
                    
                    if (!message.empty()) {
                        size_t clientIndex = i;
                        handleClientMessage(server, clientFd, message, &clientIndex);
                    }
                }
            }
        }
    }

    // Close server socket
    close(serverSocket);
    return 0;
} 