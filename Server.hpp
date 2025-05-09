#pragma once

#include <poll.h>
#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include "Channel.hpp"
#include "Client.hpp"

class Server {
public:
    Server(int port, std::string password, bool debugMode);
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();

    void run();
    void acceptClient();
    void receiveData(int clientFd, size_t index);
    void dispatchCommand(const std::string& fullMessage, int clientFd);

    void addClient(const Client& client);
    void eraseClient(int clientFd, size_t* clientIndex); // Keep only one declaration.

    size_t getClientIndex(int clientFd);
    Client* getClientObjByFd(int fd);
    Client* getClientObjByNick(const std::string& nick);
    std::vector<Client> getClients() const {
        return std::vector<Client>(_clients.begin(), _clients.end());
    }

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
    std::vector<Channel>& getChannels();
    Channel* findChannel(const std::string& name);
    bool channelExists(const std::string& name);
    void createChannel(const std::string& name, Client* creator);
    void removeEmptyChannels();
    void removeClientFromChannels(int clientFd);

    // Channel commands
    void handleJoin(int clientFd, const std::string& arg);
    void handlePart(int clientFd, const std::string& arg);
    void handleInvite(int clientFd, const std::string& arg);
    void handleKick(int clientFd, const std::string& arg);
    void handleTopic(int clientFd, const std::string& arg);
    void handleMode(int clientFd, const std::string& arg);

    // MODE
    void setMode(int clientFd, std::vector<std::string>& params);
    bool applyChannelMode(Client* client, Channel& channel,
                          const std::string& flag,
                          const std::vector<std::string>& params);

    // Client disconnect helper
    void handleClientDisconnect(int clientFd, size_t* clientIndex);
    
    // Debug helpers
    bool isDebugMode() const { return _debugMode; }
    void setDebugMode(bool mode) { _debugMode = mode; }
    void debugLog(const std::string& msg) const;

private:
    int _server_fd;
    int _port;
    std::string _password;

    std::deque<Client> _clients;
    std::vector<pollfd> _poll_fds;
    std::vector<Channel> _channels;
    std::unordered_map<int, std::string> _recvBuffers;

    // New unique client ID counter.
    int _nextClientId;

    // Debug mode flag
    bool _debugMode;

    void parser(std::string arg, std::vector<std::string>& params, char del);
};
