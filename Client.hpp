#pragma once

#include <netinet/in.h>

#include <string>

class Client {
public:
    Client();
    Client(int fd, const std::string& ip);  // <- for tests
    Client(int fd, const sockaddr_in& addr);
    Client(const Client& other);
    Client& operator=(const Client& other);
    ~Client();

    // Getters
    int getFd() const;
    bool isRegistered() const;
    const std::string& getPassword() const;
    const std::string& getNick() const;
    const std::string& getUser() const;
    const std::string& getIPa() const;

    // Setters
    void setAsRegistered();
    void setPassword(const std::string& password);
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setFd(int fd);

    // KEY
    void setKey(const std::string& k) { _key = k; }
    const std::string& getModeKey() const { return _key; }
    bool isKeyed() const { return !_key.empty(); }

    // INVISIBLE for user mode +/-i (irssi)
    void setInvisible(bool value);
    bool isInvisible() const;

private:
    std::string _nickname;
    std::string _password;
    std::string _username;
    std::string _ipA;
    int _fd;
    bool _isRegistered;
    std::string _key;
    bool _isInvisible;
};
