#include "Client.hpp"

Client::Client()
    : _nickname(""), _username(""), _ipA(""), _fd(-1), _isRegistered(false) {}

Client::Client(int fd, std::string ipAddress)
    : _nickname(""),
      _username(""),
      _ipA(ipAddress),
      _fd(fd),
      _isRegistered(false) {}

Client::~Client() {}

Client::Client(const Client& other)
    : _nickname(other.getNick()),
      _username(other.getUser()),
      _ipA(other.getIPa()),
      _fd(other.getFd()),
      _isRegistered(other.isRegistered()) {}

Client& Client::operator=(const Client& other) {
    if (this != &other) {
        _nickname = other.getNick();
        _username = other.getUser();
        _ipA = other.getIPa();
        _fd = other.getFd();
        _isRegistered = other.isRegistered();
    }
    return *this;
}
