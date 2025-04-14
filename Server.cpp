#include "Server.hpp"
#include "commands/nick.hpp"
#include "utils.hpp"
#include "commands/join.hpp"
#include "Channel.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <stdexcept>

Server::Server(int port, std::string password) : _port(port), _password(password)
{
    // Create socket
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
    {
        perror("socket");
        throw std::runtime_error("Failed to create socket");
    }

    // Set to non-blocking
    if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl");
        throw std::runtime_error("Failed to set socket to non-blocking");
    }

    // Reuse address
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        throw std::runtime_error("Failed to set socket options");
    }

    // Bind
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        throw std::runtime_error("Failed to bind");
    }

    // Listen
    if (listen(_server_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        throw std::runtime_error("Failed to listen");
    }

    // Add listening socket to poll list
    pollfd pfd = {};
    pfd.fd = _server_fd;
    pfd.events = POLLIN;
    _poll_fds.push_back(pfd);
}

Server::Server(const Server& other) : _port(other.getPort()), _password(other.getPassword()) {}

Server::~Server()
{
    close(_server_fd);
    for (size_t i = 1; i < _poll_fds.size(); ++i) close(_poll_fds[i].fd);
}

Server& Server::operator=(const Server& other)
{
    if (this != &other)
    {
        _port = other.getPort();
        _password = other.getPassword();
    }
    return (*this);
}

void Server::run()
{
    std::cout << "Server running on port " << _port << std::endl;

    while (true)
    {
        int ret = poll(_poll_fds.data(), _poll_fds.size(), -1);
        if (ret < 0)
        {
            perror("poll");
            break;
        }

        for (size_t i = 0; i < _poll_fds.size(); ++i)
        {
            if (_poll_fds[i].revents & POLLIN)
            {
                if (_poll_fds[i].fd == _server_fd)
                    acceptClient();
                else
                    receiveData(_poll_fds[i].fd, i);
            }
        }
    }
}

void Server::acceptClient()
{
    sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_fd = accept(_server_fd, (sockaddr*)&client_addr, &len);

    if (client_fd < 0)
    {
        perror("accept");
        return;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK);  // required for non-blocking sockets

    Client new_client(client_fd, client_addr);
    addClient(new_client);

    pollfd pfd = {};
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    _poll_fds.push_back(pfd);

    std::cout << "Accepted client fd: " << client_fd << std::endl;
}

void Server::receiveData(int clientFd, size_t index)
{
    char buffer[1024];
    ssize_t bytes_read = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0)
    {
        std::cout << "Client disconnected: fd = " << clientFd << std::endl;
        close(clientFd);
        _poll_fds.erase(_poll_fds.begin() + index);
        eraseClient(clientFd, &index);
        return;
    }

    buffer[bytes_read] = '\0';
    std::string fullMessage(buffer);

    if (!isRegistered(clientFd))
        registerClient(clientFd, fullMessage, &index);
    else
        dispatchCommand(fullMessage, clientFd);
}

void Server::eraseClient(int clientFd, size_t* clientIndex)
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i].getFd() == clientFd)
        {
            _clients.erase(_clients.begin() + i);
            break;
        }
    }
    if (clientIndex && *clientIndex > 0)
        (*clientIndex)--;
}

bool Server::isRegistered(int clientFd)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getFd() == clientFd)
            return it->isRegistered();
    }
    return false;
}

bool Server::isUniqueNick(std::string nick)
{
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getNick() == nick)
            return false;
    }
    return true;
}

void Server::addClient(const Client& client) { _clients.push_back(client); }

Client* Server::getClientObjByFd(int fd)
{
    for (auto& client : _clients)
    {
        if (client.getFd() == fd)
            return &client;
    }
    return nullptr;
}

size_t Server::getClientIndex(int clientFd)
{
    for (size_t i = 0; i < _clients.size(); ++i)
    {
        if (_clients[i].getFd() == clientFd)
            return i;
    }
    throw std::runtime_error("Client with fd " + std::to_string(clientFd) + " not found");
}

void Server::dispatchCommand(const std::string& fullMessage, int clientFd)
{
    std::vector<std::string> tokens;
    parser(fullMessage, tokens, ' ');
    if (tokens.empty())
        return;

    std::string command = tokens[0];

    if (command == "NICK")
        executeNick(*this, clientFd, fullMessage);
    if (command == "JOIN")
        executeJoin(*this, clientFd, fullMessage);
}

Channel* Server::getChannelByName(const std::string& name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i].getName() == name)
            return &_channels[i];
    }
    return nullptr;
}

Channel* Server::createOrGetChannel(const std::string& name)
{
    Channel* existing = getChannelByName(name);
    if (existing)
        return existing;

    _channels.push_back(Channel(name));
    return &_channels.back();
}
