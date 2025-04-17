#pragma once

#include "../Server.hpp"
#include <string>

void executeQuit(Server& server, int clientFd, const std::string& arg);
