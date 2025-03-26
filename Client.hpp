#pragma once
#include <string>

class Client {
private:
    std::string _nickname;
    std::string _username;
    std::string _ipA;
    int _fd;
    bool _isRegistered;

public:
    Client();
    Client(int fd, std::string ipAddress);
    ~Client();
    Client(const Client& other);
    Client& operator=(const Client& other);

    // Getters
    int getFd() const { return _fd; }
    bool isRegistered() const { return _isRegistered; }
    std::string getNick() const { return _nickname; }
    std::string getUser() const { return _username; }
    std::string getIPa() const { return _ipA; }

    // Setters
    void setAsRegistered() { _isRegistered = true; }
    void setNickname(std::string nick) { _nickname = nick; }
    void setUsername(std::string user) { _username = user; }
    void setFd(int fd) { _fd = fd; }
};
