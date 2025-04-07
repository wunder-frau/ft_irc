#pragma once

#include <sys/socket.h>  // for send()

#include <cstring>  // for memset
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>  // For std::runtime_error
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

class Channel;
class Client;

class Server {
private:
    int _port;
    std::string _password;
    std::vector<Client> _clients;
    std::vector<Channel> _channels;

    void parser(std::string arg, std::vector<std::string>& params, char del);
    void eraseClient(int clientFd, size_t* clientIndex);

    // Regex flags invalid nicknames by matching:
    // 1. Names starting with disallowed chars (#, $, :, &, +, %, ~, ,)
    // 2. Names containing forbidden punctuation (, * . ? ! @ or spaces)
    // 3. Names starting with '+' followed by q, a, o, h, or v (reserved for IRC
    // modes)
    inline static const std::regex incorrectRegex{
        "^([#$:#&+%~,]\\S*|\\S*[,*.?!@ ]\\S*|\\+(q|a|o|h|v)\\S*)"};

    // Regex for validating channel names.
    // Explanation:
    //   - ^[#]       : The string must start with a '#' character.
    //   - [^\\s,^\x07]+ : Following the '#' must be one or more characters that
    //                     are not whitespace, a comma, or the control character
    //                     0x07.
    //   - $          : End of the string.
    inline static const std::regex validNameRegex{"^[#][^\\s,^\x07]+$"};

    // Regex for validating channel keys.
    // Explanation:
    //   - ^           : Start of the string.
    //   - [a-zA-Z0-9]+: One or more alphanumeric characters (letters or
    //   digits).
    //   - $           : End of the string.
    inline static const std::regex validKeyRegex{"^[a-zA-Z0-9]+$"};

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

    // NICK
    size_t getClientIndex(int clientFd);
    void broadcastAndUpdateNickname(int clientFd, size_t clientIndex,
                                    const std::string& newNick);
    void sendError(int clientFd, const std::string& errorCode,
                   const std::string& nick, const std::string& details);
    void validateNick(int clientFd, const std::string& newNick);
    void nick(int clientFd, std::string arg);

    // JOIN
    bool isClientInvited(int clientFd, std::vector<int>& invitedClients);
    bool hasClientJoined(int clientFd, std::vector<int>& jointClients);
    bool isKeyOk(int clientFd, Channel& channel, std::string key);
    bool isInviteOk(int clientFd, Channel& channel);
    bool isLimitOk(int clientFd, Channel& channel);
    bool isValidName(std::string channel);
    bool isValidKey(std::string channelKey);
    void welcome(int clientFd, Channel& channel, Client& client);
    void newChannel(int clientFd, std::string channelName,
                    std::string channelKey);
    void joinChannel(int clientFd, std::string channelName,
                     std::string channelKey);
    void handleNonExistentChannel(int clientFd, const std::string& channelName,
                                  const std::string& channelKey);
    bool validateJoinConditions(int clientFd, Channel& channel,
                                const std::string& channelKey);
    void performJoin(int clientFd, Channel& channel);

    void sendClientError(int clientFd, const std::string& errorCode,
                         const std::string& details);
    void join(int clientFd, std::string arg);

    // GETTERS
    std::string getPassword() const { return _password; }
    int getPort() const { return _port; }

    // TEST/REMOVE: method to add a client to the _clients vector
    void addClient(const Client& client);
    Client* getClientObjByFd(int fd);
    int getChannelIndex(std::string name);
};
