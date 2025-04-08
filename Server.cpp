#include "Server.hpp"

Server::Server(int port, std::string password)
    : _port(port), _password(password) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
}

Server::Server(const Server& other)
    : _port(other.getPort()), _password(other.getPassword()) {}

Server::~Server() {
#ifdef _WIN32
    WSACleanup();
#endif
}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _port = other.getPort();
        _password = other.getPassword();
    }
    return (*this);
}

// Splits 'arg' by the delimiter 'del' into the vector 'params'
void Server::parser(std::string arg, std::vector<std::string>& params,
                    char del) {
    std::stringstream ss(arg);
    std::string token;
    while (std::getline(ss, token, del)) {
        if (!token.empty())
            params.push_back(token);
    }
}

// Erase client from _clients vector based on file descriptor.
void Server::eraseClient(int clientFd, size_t* clientIndex) {
    (void)clientIndex;
    std::cout << "Erasing client with FD " << clientFd << std::endl;

    // Remove client from all channels
    removeClientFromChannels(clientFd);
    
    // Find the client index
    size_t index = 0;
    for (auto it = _clients.begin(); it != _clients.end(); ++it, ++index) {
        if (it->getFd() == clientFd) {
            // Remove the client from _clients vector
            _clients.erase(it);
            
            // Update the client index
            if (clientIndex)
                (*clientIndex)--;
                
            break;
        }
    }
    
    // Remove empty channels
    removeEmptyChannels();
}

// Check if the client with file descriptor 'clientFd' is registered.
bool Server::isRegistered(int clientFd) {
    for (std::vector<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        if (it->getFd() == clientFd)
            return it->isRegistered();
    }
    return false;
}

// Check that 'nick' is not already in use by any connected client.
bool Server::isUniqueNick(std::string nick) {
    for (std::vector<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        if (it->getNick() == nick)
            return false;
    }
    return true;
}

// TEST/REMOVE: add a client to the server.
void Server::addClient(const Client& client) { _clients.push_back(client); }

Client* Server::getClientObjByFd(int fd) {
    for (auto& client : _clients) {
        if (client.getFd() == fd)
            return &client;
    }
    return nullptr;
}
