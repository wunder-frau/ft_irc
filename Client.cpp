#include "Client.hpp"

#include <arpa/inet.h>

#include <iostream>

Client::Client()
    : _nickname(""),
      _password(""),
      _username(""),
      _ipA(""),
      _fd(-1),
      _isRegistered(false),
      _key(""),
      _isInvisible(false) {}

Client::Client(int fd, const std::string& ip)
    : _ipA(ip), _fd(fd), _isRegistered(false), _key(""), _isInvisible(false) {}

Client::Client(int fd, const sockaddr_in& addr)
    : _nickname(""),
      _password(""),
      _username(""),
      _fd(fd),
      _isRegistered(false),
      _key(""),
      _isInvisible(false) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    _ipA = ip;
}

Client::~Client() {}

Client::Client(const Client& other)
    : _nickname(other.getNick()),
      _password(other.getPassword()),
      _username(other.getUser()),
      _ipA(other.getIPa()),
      _fd(other.getFd()),
      _isRegistered(other.isRegistered()),
      _key(other.getModeKey()),
      _isInvisible(other.isInvisible()) {}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _nickname = other.getNick();
        _password = other.getPassword();
        _username = other.getUser();
        _ipA = other.getIPa();
        _fd = other.getFd();
        _isRegistered = other.isRegistered();
        _key = other.getModeKey();
        _isInvisible = other.isInvisible();
    }
    return *this;
}

int Client::getFd() const { return _fd; }

bool Client::isRegistered() const { return _isRegistered; }

const std::string& Client::getPassword() const { return _password; }

const std::string& Client::getNick() const { return _nickname; }

const std::string& Client::getUser() const { return _username; }

const std::string& Client::getIPa() const { return _ipA; }

void Client::setAsRegistered() {
    _isRegistered = true;
    std::cout << "[INFO] Client fd=" << _fd << " marked as registered"
              << std::endl;
}

void Client::setPassword(const std::string& password) {
    _password = password;
    std::cout << "[INFO] Client fd=" << _fd << " set password" << std::endl;
}

void Client::setNickname(const std::string& nick) {
    std::string trimmedNick = nick;
    while (!trimmedNick.empty() &&
           (trimmedNick.back() == '\n' || trimmedNick.back() == '\r' ||
            trimmedNick.back() == ' ' || trimmedNick.back() == '\t')) {
        trimmedNick.pop_back();
    }

    _nickname = trimmedNick;
    std::cout << "[INFO] Client fd=" << _fd << " set nickname: " << trimmedNick
              << std::endl;
}

void Client::setUsername(const std::string& user) {
    std::string trimmedUser = user;
    while (!trimmedUser.empty() &&
           (trimmedUser.back() == '\n' || trimmedUser.back() == '\r' ||
            trimmedUser.back() == ' ' || trimmedUser.back() == '\t')) {
        trimmedUser.pop_back();
    }

    _username = trimmedUser;
    std::cout << "[INFO] Client fd=" << _fd << " set username: " << trimmedUser
              << std::endl;
}

void Client::setFd(int fd) { _fd = fd; }

// INVISIBLE for user mode +/-i (irssi)

void Client::setInvisible(bool value) { _isInvisible = value; }

bool Client::isInvisible() const { return _isInvisible; }
