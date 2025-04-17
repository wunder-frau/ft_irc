#pragma once

#include <string>

class Server;

void executeNick(Server& server, int clientFd, const std::string& arg);
