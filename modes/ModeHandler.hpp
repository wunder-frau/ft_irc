#pragma once
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

bool handleInviteOnlyMode(Client* client, Channel& channel, bool adding);
bool handleTopicRestrictMode(Client* client, Channel& channel, bool adding);
bool handleKeyMode(Client* client, Channel& channel, bool adding,
                   const std::vector<std::string>& params);
bool handleLimitMode(Client* client, Channel& channel, bool adding,
                     const std::vector<std::string>& params);
bool handleOpMode(Server& server, Client* client, Channel& channel, bool adding,
                  const std::vector<std::string>& params);
