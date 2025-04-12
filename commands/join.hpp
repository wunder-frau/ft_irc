#pragma once

#include <string>

class Server;

// Entry point for handling the JOIN command
void executeJoin(Server& server, int clientFd, const std::string& arg);
