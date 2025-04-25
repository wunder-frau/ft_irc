#include "../Server.hpp"
#include <sys/socket.h>

void executePing(Server& server, int clientFd, const std::string& arg)
{
    (void)server;
    size_t pos = arg.find("PING");
    std::string token = (pos != std::string::npos) ? arg.substr(pos + 4) : "";
    std::string response = "PONG" + token + "\r\n";
    send(clientFd, response.c_str(), response.length(), 0);
}
