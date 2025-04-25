#include <algorithm>
#include <sstream>

#include "../Server.hpp"
#include "../utils.hpp"

static void deliverMessage(Server& server, int senderFd,
                           const std::string& targetName,
                           const std::string& message,
                           const std::string& command) {
    Client* sender = server.getClientObjByFd(senderFd);
    if (!sender)
        return;

    std::string prefix = ":" + sender->getNick() + "!~" + sender->getUser() +
                         "@" + sender->getIPa();

    std::string trimmedTarget = trimWhitespace(targetName);
    std::string normalizedTarget = normalizeChannelName(trimmedTarget);

    // Check if target is a user
    Client* targetClient = server.getClientObjByNick(trimmedTarget);
    if (targetClient) {
        std::string fullMessage = prefix + " " + command + " " + trimmedTarget +
                                  " :" + message + "\r\n";
        send(targetClient->getFd(), fullMessage.c_str(), fullMessage.length(),
             0);
        return;
    }

    // Else, maybe it's a channel
    Channel* channel = server.findChannel(normalizedTarget);
    if (channel) {
        std::string fullMessage = prefix + " " + command + " " +
                                  channel->getName() + " :" + message + "\r\n";
        for (Client* member : channel->getClients()) {
            if (member->getFd() != senderFd) {
                send(member->getFd(), fullMessage.c_str(), fullMessage.length(),
                     0);
            }
        }
        return;
    }

    // If not found and it's PRIVMSG, send error
    if (command == "PRIVMSG") {
        sendError(senderFd, "401", sender->getNick(),
                  trimmedTarget + " :No such nick/channel");
    }
}

void executeNotice(Server& server, int clientFd,
                   const std::string& fullMessage) {
    std::vector<std::string> tokens;
    parser(fullMessage, tokens, ' ');

    if (tokens.size() < 3)
        return;  // No reply or error for NOTICE

    std::string target = trimWhitespace(tokens[1]);
    std::string message;

    size_t colonPos = fullMessage.find(':');
    if (colonPos != std::string::npos)
        message = fullMessage.substr(colonPos + 1);

    deliverMessage(server, clientFd, target, message, "NOTICE");
}
