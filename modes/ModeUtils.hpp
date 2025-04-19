#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

class Server;

bool isValidKey(const std::string& s);
bool setKey(Server& server, int clientFd, Channel& channel,
            const std::vector<std::string>& params);
bool hasOpRights(Server& server, int clientFd, const std::string& channelName);
bool isClient(Server& server, const std::string& nick);
int getChannelIndex(Server& server, const std::string& name);
bool verifyParams(Server& server, int clientFd,
                  std::vector<std::string>& params);
void returnChannelMode(Server& server, int clientFd, Channel& channel);
