#include "../Server.hpp"

void join(Server& server, int clientFd, const std::string& arg)
{
    server.handleJoin(clientFd, arg);
}

void executeJoin(Server& server, int clientFd, const std::string& arg)
{
    join(server, clientFd, arg);
}
