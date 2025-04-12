#include "Client.hpp"
#include <arpa/inet.h>
#include <iostream>

Client::Client()
    : _nickname(""), _password(""), _username(""), _ipA(""), _fd(-1), _isRegistered(false)
{
}

Client::Client(int fd, const sockaddr_in& addr)
    : _nickname(""), _password(""), _username(""), _fd(fd), _isRegistered(false)
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    _ipA = ip;

    std::cout << "[INFO] New Client created: fd=" << _fd << ", ip=" << _ipA << std::endl;
}

Client::~Client() {}

Client::Client(const Client& other)
    : _nickname(other.getNick()),
      _password(other.getPassword()),
      _username(other.getUser()),
      _ipA(other.getIPa()),
      _fd(other.getFd()),
      _isRegistered(other.isRegistered())
{
}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
    {
        _nickname = other.getNick();
        _password = other.getPassword();
        _username = other.getUser();
        _ipA = other.getIPa();
        _fd = other.getFd();
        _isRegistered = other.isRegistered();
    }
    return *this;
}

int Client::getFd() const { return _fd; }

bool Client::isRegistered() const { return _isRegistered; }

const std::string& Client::getPassword() const { return _password; }

const std::string& Client::getNick() const { return _nickname; }

const std::string& Client::getUser() const { return _username; }

const std::string& Client::getIPa() const { return _ipA; }

void Client::setAsRegistered()
{
    _isRegistered = true;
    std::cout << "[INFO] Client fd=" << _fd << " marked as registered" << std::endl;
}

void Client::setPassword(const std::string& password)
{
    _password = password;
    std::cout << "[INFO] Client fd=" << _fd << " set password" << std::endl;
}

void Client::setNickname(const std::string& nick)
{
    _nickname = nick;
    std::cout << "[INFO] Client fd=" << _fd << " set nickname: " << nick << std::endl;
}

void Client::setUsername(const std::string& user)
{
    _username = user;
    std::cout << "[INFO] Client fd=" << _fd << " set username: " << user << std::endl;
}

void Client::setFd(int fd) { _fd = fd; }
