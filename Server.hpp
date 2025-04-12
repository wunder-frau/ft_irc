#pragma once

#include <string>
#include <vector>
#include <poll.h>

#include "Client.hpp"
#include "Channel.hpp"

class Server
{
   public:
    Server(int port, std::string password);
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();

    void run();
    void acceptClient();
    void receiveData(int clientFd, size_t index);
    void dispatchCommand(const std::string& fullMessage, int clientFd);

    void addClient(const Client& client);
    void eraseClient(int clientFd, size_t* clientIndex);

    size_t getClientIndex(int clientFd);
    Client* getClientObjByFd(int fd);

    bool isRegistered(int clientFd);
    bool isUniqueNick(std::string nick);

    inline int getPort() const { return _port; }
    inline std::string getPassword() const { return _password; }

    void registerClient(int clientFd, const std::string& arg, size_t* clientIndex);
    void registerPassword(Client& client, const std::string& arg, size_t* clientIndex);
    void registerNickname(Client& client, const std::string& arg);
    void registerUser(Client& client, const std::string& arg);
    void authenticate(Client& client, const std::string& arg, size_t* clientIndex);

    // Channel management
    Channel* getChannelByName(const std::string& name);
    Channel* createOrGetChannel(const std::string& name);

   private:
    int _server_fd;
    int _port;
    std::string _password;

    std::vector<Client> _clients;
    std::vector<pollfd> _poll_fds;
    std::vector<Channel> _channels;
};
