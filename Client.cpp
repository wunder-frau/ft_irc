#include "Client.hpp"

Client::Client()
    : _nickname(""),
      _password(""),
      _username(""),
      _ipA(""),
      _fd(-1),
      _isRegistered(false) {}

Client::Client(int fd, std::string ipAddress)
    : _nickname(""),
      _password(""),
      _username(""),
      _ipA(ipAddress),
      _fd(fd),
      _isRegistered(false) {}

Client::~Client() {}

Client::Client(const Client& other)
    : _nickname(other.getNick()),
      _password(other.getPassword()),
      _username(other.getUser()),
      _ipA(other.getIPa()),
      _fd(other.getFd()),
      _isRegistered(other.isRegistered()) {}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
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

std::string Client::getPassword() const { return _password; }

std::string Client::getNick() const { return _nickname; }

std::string Client::getUser() const { return _username; }

std::string Client::getIPa() const { return _ipA; }

void Client::setAsRegistered() { _isRegistered = true; }

void Client::setPassword(std::string password) { _password = password; }

void Client::setNickname(std::string nick) { _nickname = nick; }

void Client::setUsername(std::string user) { _username = user; }

void Client::setFd(int fd) { _fd = fd; }
