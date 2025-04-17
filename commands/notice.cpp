#include "../Server.hpp"
#include "../utils.hpp"
#include <algorithm>
#include <sstream>

static void deliverMessage(Server& server, int senderFd, const std::string& targetName,
                           const std::string& message, const std::string& command)
{
    Client* sender = server.getClientObjByFd(senderFd);
    if (!sender)
        return;

    std::string prefix =
        ":" + sender->getNick() + "!~" + sender->getUser() + "@" + sender->getIPa();
    std::string fullMessage = prefix + " " + command + " " + targetName + " :" + message + "\r\n";

    // Send to user
    Client* targetClient = server.getClientObjByNick(targetName);
    if (targetClient)
    {
        send(targetClient->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
        return;
    }

    // Send to channel
    Channel* channel = server.findChannel(targetName);
    if (channel)
    {
        for (Client* member : channel->getClients())
        {
            if (member->getFd() != senderFd)
            {
                send(member->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
            }
        }
        return;
    }

    // If target is not found and command is PRIVMSG, send error
    if (command == "PRIVMSG")
    {
        sendError(senderFd, "401", sender->getNick(), targetName + " :No such nick/channel");
    }
    // If command is NOTICE, do nothing on error
}

void executeNotice(Server& server, int clientFd, const std::string& fullMessage)
{
    std::vector<std::string> tokens;
    parser(fullMessage, tokens, ' ');

    if (tokens.size() < 3)
        return;  // No reply or error for NOTICE

    std::string target = tokens[1];
    // Trim whitespace from target
    target = trimWhitespace(target);
    
    std::string message;

    size_t colonPos = fullMessage.find(':');
    if (colonPos != std::string::npos)
        message = fullMessage.substr(colonPos + 1);

    deliverMessage(server, clientFd, target, message, "NOTICE");
}
