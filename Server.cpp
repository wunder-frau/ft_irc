#include "Server.hpp"

#include <arpa/inet.h>  // <-- Needed for inet_ntoa
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <deque>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Channel.hpp"
#include "commands/join.hpp"
#include "commands/mode.hpp"
#include "commands/nick.hpp"
#include "commands/notice.hpp"
#include "commands/privmsg.hpp"
#include "commands/quit.hpp"
#include "utils.hpp"

Server::Server(int port, std::string password, bool debugMode)
    : _port(port), _password(password), _nextClientId(0), _debugMode(debugMode)
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
    {
        perror("socket");
        throw std::runtime_error("Failed to create socket");
    }

    if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl");
        throw std::runtime_error("Failed to set socket to non-blocking");
    }

    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        throw std::runtime_error("Failed to set socket options");
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        throw std::runtime_error("Failed to bind");
    }

    if (listen(_server_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        throw std::runtime_error("Failed to listen");
    }

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
    return *this;
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

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    _clients.emplace_back(client_fd, client_addr);

    pollfd pfd = {};
    pfd.fd = client_fd;
    pfd.events = POLLIN;
    _poll_fds.push_back(pfd);

    std::cout << "[INFO] New Client created: fd=" << client_fd
              << ", ip=" << inet_ntoa(client_addr.sin_addr) << std::endl;
    std::cout << "Accepted client fd: " << client_fd << std::endl;

    // Send welcome message to the connecting client
    std::string welcome = "Welcome to the IRC server. please provide PASS, USER, NICK:\r\n";
    send(client_fd, welcome.c_str(), welcome.length(), 0);
}

void Server::receiveData(int clientFd, size_t index)
{
    char buffer[1024];
    ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        std::cout << "[INFO] Client disconnected: fd=" << clientFd << std::endl;
        close(clientFd);
        _recvBuffers.erase(clientFd);
        _poll_fds.erase(_poll_fds.begin() + index);
        eraseClient(clientFd, &index);
        return;
    }

    auto& pending = _recvBuffers[clientFd];
    pending.append(buffer, n);
    size_t pos;
    while ((pos = pending.find('\n')) != std::string::npos)
    {
        std::string line = pending.substr(0, pos);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        pending.erase(0, pos + 1);
        if (!isRegistered(clientFd))
        {
            registerClient(clientFd, line, &index);
        }
        else
        {
            dispatchCommand(line, clientFd);
        }
    }
}

void Server::eraseClient(int clientFd, size_t* clientIndex)
{
    (void)clientIndex;
    debugLog("Erasing client with FD " + std::to_string(clientFd));
    // Find the client index
    size_t index = 0;
    for (auto it = _clients.begin(); it != _clients.end(); ++it, ++index)
    {
        if (it->getFd() == clientFd)
        {
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

bool Server::isRegistered(int clientFd)
{
    for (auto it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getFd() == clientFd)
            return it->isRegistered();
    }
    return false;
}

bool Server::isUniqueNick(std::string nick)
{
    for (auto it = _clients.begin(); it != _clients.end(); ++it)
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

Client* Server::getClientObjByNick(const std::string& nick)
{
    for (auto it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->getNick() == nick)
            return &(*it);
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
    std::istringstream stream(fullMessage);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (!line.empty() && line[0] == ':')
            continue;

        std::vector<std::string> tokens;
        parser(line, tokens, ' ');
        if (tokens.empty())
            continue;

        //        const std::string& command = tokens[0];
        std::string command = toUpperCase(tokens[0]);

        if (command == "NICK")
        {
            executeNick(*this, clientFd, line);
        }
        else if (command == "JOIN")
        {
            handleJoin(clientFd, line);
        }
        else if (command == "PART")
        {
            handlePart(clientFd, line);
        }
        else if (command == "PRIVMSG" || command == "MSG")
        {
            executePrivmsg(*this, clientFd, line);
        }
        else if (command == "NOTICE")
        {
            executeNotice(*this, clientFd, line);
        }
        else if (command == "QUIT")
        {
            executeQuit(*this, clientFd, line);
        }
        else if (command == "MODE")
        {
            executeMode(*this, clientFd, line);
        }
        else if (command == "TOPIC")
        {
            handleTopic(clientFd, line);
        }
        else if (command == "KICK")
        {
            handleKick(clientFd, line);
        }
        else if (command == "INVITE")
        {
            handleInvite(clientFd, line);
        }
        else
        {
            sendError(clientFd, "421", command, ":Unknown command");
        }
    }
}

std::vector<Channel>& Server::getChannels() { return _channels; }

Channel* Server::getChannelByName(const std::string& name)
{
    std::string searchKey = normalizeChannelName(trimWhitespace(name));

    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (_channels[i].getNormalizedName() == searchKey)
            return &_channels[i];
    }
    return nullptr;
}

Channel* Server::createOrGetChannel(const std::string& name)
{
    std::string trimmed = trimWhitespace(name);
    Channel* existing = getChannelByName(trimmed);
    if (existing)
        return existing;

    _channels.push_back(Channel(trimmed));  // Channel constructor will store normalized key
    return &_channels.back();
}

// Helper method to safely disconnect a client
void Server::handleClientDisconnect(int clientFd, size_t* clientIndex)
{
    std::cout << "Disconnecting client with FD " << clientFd << std::endl;

    // First remove client from all channels
    removeClientFromChannels(clientFd);

    // Then erase the client from the _clients vector
    eraseClient(clientFd, clientIndex);
}

#include <sstream>

// Implementation of the parser function
void Server::parser(std::string arg, std::vector<std::string>& params, char del)
{
    std::istringstream iss(arg);
    std::string token;
    while (std::getline(iss, token, del))
    {
        if (!token.empty())
            params.push_back(token);
    }
}

void Server::debugLog(const std::string& msg) const
{
    if (_debugMode)
    {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
}
