#include "mode.hpp"

#include "../Server.hpp"

void mode(Server& server, int clientFd, const std::string& arg) {
    server.handleMode(clientFd, arg);
}

void executeMode(Server& server, int clientFd, const std::string& arg) {
    mode(server, clientFd, arg);
}
