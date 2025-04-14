#pragma once

#include "../Server.hpp"
#include <string>

// Handles the PRIVMSG command from the client.
void executePrivmsg(Server& server, int clientFd, const std::string& arg);
