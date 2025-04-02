#pragma once

#include <sys/socket.h>  // for send()

#include <cstring>  // for memset
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "Client.hpp"

class Server {
private:
    int _port;
    std::string _password;
    std::vector<Client> _clients;

    void parser(std::string arg, std::vector<std::string>& params, char del);
    void eraseClient(int clientFd, size_t* clientIndex);

    // Regex flags invalid nicknames by matching:
    // 1. Names starting with disallowed chars (#, $, :, &, +, %, ~, ,)
    // 2. Names containing forbidden punctuation (, * . ? ! @ or spaces)
    // 3. Names starting with '+' followed by q, a, o, h, or v (reserved for IRC
    // modes)
    inline static const std::regex incorrectRegex{
        "^([#$:#&+%~,]\\S*|\\S*[,*.?!@ ]\\S*|\\+(q|a|o|h|v)\\S*)"};

public:
    Server(int port, std::string password);
    ~Server();
    Server(const Server& other);
    Server& operator=(const Server& other);

    // CLIENT: Registration Methods
    bool isRegistered(int clientFd);
    void registerPassword(Client& client, std::string arg, size_t* clientIndex);
    bool isUniqueNick(std::string nick);
    void registerNickname(Client& client, std::string arg);
    void registerUser(Client& client, std::string arg);
    void authenticate(Client& client, std::string arg, size_t* clientIndex);
    void registerClient(int clientFd, std::string arg, size_t* clientIndex);

    std::string getPassword() const { return _password; }
    int getPort() const { return _port; }

    // TEST/REMOVE: method to add a client to the _clients vector
    void addClient(const Client& client);
    Client* getClientObjByFd(int fd);
};
