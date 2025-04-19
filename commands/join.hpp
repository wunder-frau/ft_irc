#pragma once

#include "../Server.hpp"
#include <string>

/// Handles the IRC JOIN command.
/// Parses parameters and delegates to Server::joinChannel.
void join(Server& server, int clientFd, const std::string& arg);
void executeJoin(Server& server, int clientFd, const std::string& arg);
