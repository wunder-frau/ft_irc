#include "utils.hpp"
#include "Server.hpp"

#include <sstream>
#include <sys/socket.h>  // for send()

void parser(const std::string& input, std::vector<std::string>& output, char delimiter)
{
    std::stringstream ss(input);
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        if (!token.empty())
            output.push_back(token);
    }
}

void sendError(Server& server, int clientFd, const std::string& errorCode, const std::string& nick,
               const std::string& details)
{
    (void)server;  // currently unused
    sendError(clientFd, errorCode, nick, details);
}

void sendError(int clientFd, const std::string& errorCode, const std::string& nick,
               const std::string& details)
{
    std::string msg = ":ft_irc " + errorCode + " " + nick + " " + details + "\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);
}
