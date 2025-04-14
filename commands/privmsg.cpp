#include "../Server.hpp"
#include "../utils.hpp"

void executePrivmsg(Server& server, int clientFd, const std::string& fullMessage)
{
    std::vector<std::string> tokens;
    parser(fullMessage, tokens, ' ');

    Client* sender = server.getClientObjByFd(clientFd);
    if (!sender || tokens.size() < 3)
    {
        if (sender)
            sendError(clientFd, "411", sender->getNick(), ":No recipient given (PRIVMSG)");
        return;
    }

    std::string target = tokens[1];
    std::string message = fullMessage.substr(fullMessage.find(tokens[2]));

    // Check if target is a channel
    if (!target.empty() && target[0] == '#')
    {
        Channel* channel = server.findChannel(target);
        if (!channel)
        {
            sendError(clientFd, "403", sender->getNick(), target + " :No such channel");
            return;
        }

        if (!channel->isInChannel(sender))
        {
            sendError(clientFd, "404", sender->getNick(), target + " :Cannot send to channel");
            return;
        }

        std::string msg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" +
                          sender->getIPa() + " PRIVMSG " + target + " :" + message + "\r\n";
        channel->broadcast(msg, sender);
    }
    else
    {
        Client* recipient = server.getClientObjByNick(target);
        if (!recipient)
        {
            sendError(clientFd, "401", sender->getNick(), target + " :No such nick");
            return;
        }

        std::string msg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" +
                          sender->getIPa() + " PRIVMSG " + target + " :" + message + "\r\n";
        send(recipient->getFd(), msg.c_str(), msg.length(), 0);
    }
}
