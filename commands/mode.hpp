#pragma once

#include <string>

#include "../Server.hpp"

void mode(Server& server, int clientFd, const std::string& arg);

void executeMode(Server& server, int clientFd, const std::string& arg);
