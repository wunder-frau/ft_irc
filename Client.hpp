#pragma once
#include <string>

class Client {
private:
    std::string _nickname;
    std::string _password;
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

    int getFd() const;
    bool isRegistered() const;
    std::string getPassword() const;
    std::string getNick() const;
    std::string getUser() const;
    std::string getIPa() const;

    void setAsRegistered();
    void setPassword(std::string password);
    void setNickname(std::string nick);
    void setUsername(std::string user);
    void setFd(int fd);
};
