#pragma once

#include <string>

class Server;
void executePing(Server& server, int clientFd, const std::string& arg);
