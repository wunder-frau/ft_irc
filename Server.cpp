#include "Server.hpp"

Server::Server(int port, std::string password)
    : _port(port), _password(password) {}

Server::Server(const Server& other)
    : _port(other.getPort()), _password(other.getPassword()) {}

Server::~Server() {}

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
    // TODO:
    //
    // 1. Retrieve the client index:
    //    - Implement getClientIndex(clientFd) to iterate through _clients and
    //    return the index of the client with the given file descriptor.
    //    - If no client is found, return -1.
    //
    // 2. Remove the client from channels:
    //    - Call removeClientFromChannels(clientFd) to remove the client from
    //    every channel's list of active users and operators.
    //
    // 3. Remove the client from the _clients vector:
    //    - Use the index from step 1 to erase the client from _clients.
    //
    // 4. Update the pollfd vector (_fds):
    //    - Remove the corresponding pollfd entry for the client using
    //    clientIndex.
    //
    // 5. Close the client's socket:
    //    - Call close(clientFd) to properly close the socket and free up
    //    resources.
    //
    // 6. Remove any dead channels:
    //    - Implement removeDeadChannels() to iterate over _channels and remove
    //    channels that no longer have any active clients.
    //
    // 7. Update the client index:
    //    - Decrement *clientIndex to reflect the removal of the client.
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
