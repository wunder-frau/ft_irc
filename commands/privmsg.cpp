#include "../Server.hpp"
#include "../utils.hpp"
#include <cctype>  // for std::tolower
#include <iostream>

void executePrivmsg(Server& server, int clientFd, const std::string& fullMessage)
{
    try {
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

        // Debug logs
        std::cout << "[DEBUG] PRIVMSG - Sender: '" << sender->getNick() << "', Target: '" << target << "'" << std::endl;
        
        // Trim whitespace from target using the common function
        target = trimWhitespace(target);
        
        std::cout << "[DEBUG] PRIVMSG - After trimming, Target: '" << target << "'" << std::endl;

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

            // Create the message with trimmed nickname and username
            std::string nickname = sender->getNick();
            std::string username = sender->getUser();
            std::string ipAddress = sender->getIPa();
            
            // Make sure we have valid values
            if (nickname.empty()) nickname = "unknown";
            if (username.empty()) username = "unknown";
            if (ipAddress.empty()) ipAddress = "127.0.0.1";
            
            std::string msg = ":" + nickname + "!~" + username + "@" +
                            ipAddress + " PRIVMSG " + target + " :" + message + "\r\n";
            
            // Use a try-catch block to catch any errors during broadcast
            try {
                channel->broadcast(msg, sender);
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] in channel broadcast: " << e.what() << std::endl;
                sendError(clientFd, "421", sender->getNick(), "PRIVMSG :Internal server error");
            }
        }
        else
        {
            // Debug logs
            std::cout << "[DEBUG] PRIVMSG - Looking for client with nick: '" << target << "'" << std::endl;
            std::cout << "[DEBUG] PRIVMSG - Sender's nick: '" << sender->getNick() << "'" << std::endl;
            
            // Print all client nicknames for debugging
            std::cout << "[DEBUG] PRIVMSG - All client nicknames:" << std::endl;
            const std::vector<Client>& clients = server.getClients();
            for (size_t i = 0; i < clients.size(); ++i)
            {
                std::cout << "[DEBUG]   " << i << ": '" << clients[i].getNick() << "'" << std::endl;
                
                // Special check for self-messaging - if this is the sender
                if (clients[i].getFd() == clientFd)
                {
                    std::cout << "[DEBUG]   This is the sender (self)" << std::endl;
                    
                    // If target matches sender's nick (case-insensitive)
                    std::string senderNick = clients[i].getNick();
                    std::string targetLower = target;
                    std::string senderLower = senderNick;
                    
                    for (size_t j = 0; j < targetLower.size(); j++) {
                        targetLower[j] = std::tolower(targetLower[j]);
                    }
                    for (size_t j = 0; j < senderLower.size(); j++) {
                        senderLower[j] = std::tolower(senderLower[j]);
                    }
                    
                    if (targetLower == senderLower)
                    {
                        std::cout << "[DEBUG]   Detected self-message!" << std::endl;
                        // For self-messages, use the sender as recipient
                        std::string msg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" +
                                        sender->getIPa() + " PRIVMSG " + target + " :" + message + "\r\n";
                        send(clientFd, msg.c_str(), msg.length(), 0);
                        return;
                    }
                }
            }

            // Try to find recipient
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
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] in executePrivmsg: " << e.what() << std::endl;
        // Try to send a generic error if we can
        try {
            Client* sender = server.getClientObjByFd(clientFd);
            if (sender) {
                sendError(clientFd, "421", sender->getNick(), "PRIVMSG :Internal server error");
            }
        } catch (...) {
            // Ignore any errors in error handling
        }
    }
}
