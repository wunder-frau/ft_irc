#include "nick.hpp"
#include "../Server.hpp"
#include "../utils.hpp"

#include <regex>
#include <iostream>

extern std::regex incorrectRegex;

// Broadcast nickname change and update the client
static void broadcastAndUpdateNickname(Server& server, int clientFd, const std::string& newNick)
{
    Client* client = server.getClientObjByFd(clientFd);  // safe access
    std::string currentNick = client->getNick();

    std::string msg = ":" + currentNick + "!~" + client->getUser() + "@" + client->getIPa() +
                      " NICK " + newNick + "\r\n";

    send(clientFd, msg.c_str(), msg.length(), 0);
    client->setNickname(newNick);
}

// Validate and apply nickname if it's allowed
static void validateNick(Server& server, int clientFd, const std::string& newNick)
{
    Client* client = server.getClientObjByFd(clientFd);  // no direct vector access
    std::string currentNick = client->getNick();

    // First, trim whitespace from the nickname
    std::string trimmedNick = trimWhitespace(newNick);

    if (trimmedNick.empty())
    {
        sendError(server, clientFd, "432", currentNick, newNick + " :Erroneous nickname (empty)");
        return;
    }

    if (trimmedNick.find(' ') != std::string::npos)
    {
        sendError(server, clientFd, "432", currentNick,
                  trimmedNick + " :Erroneous nickname (contains space)");
        return;
    }

    if (std::regex_match(trimmedNick, incorrectRegex))
    {
        sendError(server, clientFd, "432", currentNick,
                  trimmedNick + " :Erroneous nickname (invalid pattern)");
        return;
    }

    if (!server.isUniqueNick(trimmedNick))
    {
        sendError(server, clientFd, "433", currentNick,
                  trimmedNick + " :Nickname is already in use");
        return;
    }

    broadcastAndUpdateNickname(server, clientFd, trimmedNick);
}

// Entry point for NICK command
void executeNick(Server& server, int clientFd, const std::string& arg)
{
    std::vector<std::string> params;
    parser(arg, params, ' ');  // now using utils.hpp parser

    if (params.empty() || params.at(0) != "NICK")
    {
        Client* client = server.getClientObjByFd(clientFd);
        sendError(server, clientFd, "421", client->getUser(), ":Unknown command");
        return;
    }

    if (params.size() <= 1)
    {
        Client* client = server.getClientObjByFd(clientFd);
        sendError(server, clientFd, "431", client->getNick(), ":No nickname given");
        return;
    }

    Client* client = server.getClientObjByFd(clientFd);
    std::string trimmedNick = trimWhitespace(params.at(1));
    if (client->getNick() == trimmedNick)
        return;

    validateNick(server, clientFd, trimmedNick);
}
