#include <cctype>  // for std::tolower
#include <iostream>
#include <set>

#include "../Server.hpp"
#include "../utils.hpp"

void executePrivmsg(Server& server, int clientFd, const std::string& fullMessage)
{
    try
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

        std::string rawTargets = tokens[1];
        std::vector<std::string> targetList;
        parser(rawTargets, targetList, ',');

        std::string message = fullMessage.substr(fullMessage.find(tokens[2]));

        std::set<int> sentFds;  // to avoid duplicate sends

        for (size_t t = 0; t < targetList.size(); ++t)
        {
            std::string target = trimWhitespace(targetList[t]);

            server.debugLog("PRIVMSG - Sender: '" + sender->getNick() + "', Target: '" + target +
                            "'");
            server.debugLog("PRIVMSG - After trimming, Target: '" + target + "'");

            if (!target.empty() && target[0] == '#')
            {
                Channel* channel = server.findChannel(target);
                if (!channel)
                {
                    sendError(clientFd, "403", sender->getNick(), target + " :No such channel");
                    continue;
                }

                if (!channel->isInChannel(sender))
                {
                    sendError(clientFd, "404", sender->getNick(),
                              target + " :Cannot send to channel");
                    continue;
                }

                std::string nickname = sender->getNick();
                std::string username = sender->getUser();
                std::string ipAddress = sender->getIPa();

                if (nickname.empty())
                    nickname = "unknown";
                if (username.empty())
                    username = "unknown";
                if (ipAddress.empty())
                    ipAddress = "127.0.0.1";

                std::string msg = ":" + nickname + "!~" + username + "@" + ipAddress + " PRIVMSG " +
                                  target + " :" + message + "\r\n";

                try
                {
                    const std::vector<Client*>& clients = channel->getClients();
                    for (size_t i = 0; i < clients.size(); ++i)
                    {
                        if (clients[i] && clients[i]->getFd() != clientFd &&
                            sentFds.insert(clients[i]->getFd()).second)
                        {
                            send(clients[i]->getFd(), msg.c_str(), msg.length(), 0);
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "[ERROR] in channel broadcast: " << e.what() << std::endl;
                    sendError(clientFd, "421", sender->getNick(), "PRIVMSG :Internal server error");
                }
            }
            else
            {
                server.debugLog("PRIVMSG - Looking for client with nick: '" + target + "'");
                server.debugLog("PRIVMSG - Sender's nick: '" + sender->getNick() + "'");
                server.debugLog("PRIVMSG - All client nicknames:");

                const std::vector<Client>& clients = server.getClients();
                for (size_t i = 0; i < clients.size(); ++i)
                {
                    server.debugLog("PRIVMSG - " + std::to_string(i) + ": '" +
                                    clients[i].getNick() + "'");

                    if (clients[i].getFd() == clientFd)
                    {
                        std::string senderNick = clients[i].getNick();
                        std::string targetLower = ircCaseFold(target);
                        std::string senderLower = ircCaseFold(senderNick);

                        if (targetLower == senderLower)
                        {
                            server.debugLog("PRIVMSG - Detected self-message!");

                            if (sentFds.insert(clientFd).second)
                            {
                                std::string msg = ":" + sender->getNick() + "!~" +
                                                  sender->getUser() + "@" + sender->getIPa() +
                                                  " PRIVMSG " + target + " :" + message + "\r\n";
                                send(clientFd, msg.c_str(), msg.length(), 0);
                            }
                            continue;
                        }
                    }
                }

                Client* recipient = server.getClientObjByNick(target);
                if (!recipient)
                {
                    sendError(clientFd, "401", sender->getNick(), target + " :No such nick");
                    continue;
                }

                if (sentFds.insert(recipient->getFd()).second)
                {
                    std::string msg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" +
                                      sender->getIPa() + " PRIVMSG " + target + " :" + message +
                                      "\r\n";
                    send(recipient->getFd(), msg.c_str(), msg.length(), 0);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ERROR] in executePrivmsg: " << e.what() << std::endl;
        try
        {
            Client* sender = server.getClientObjByFd(clientFd);
            if (sender)
                sendError(clientFd, "421", sender->getNick(), "PRIVMSG :Internal server error");
        }
        catch (...)
        {
        }
    }
}
