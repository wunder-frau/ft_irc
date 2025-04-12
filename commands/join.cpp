#include <iostream>

#include "join.hpp"
#include "../utils.hpp"
#include "../Server.hpp"

// Temporary stub for JOIN command
void executeJoin(Server& server, int clientFd, const std::string& arg)
{
    std::cout << "[JOIN] Received JOIN command from fd=" << clientFd << " with args: " << arg
              << std::endl;

    // TODO: Implement real JOIN logic (parsing channel name, adding client to it)
    std::string reply = ":ft_irc 331 " + server.getClientObjByFd(clientFd)->getNick() +
                        " :JOIN command received but not implemented yet\r\n";
    send(clientFd, reply.c_str(), reply.length(), 0);
}
