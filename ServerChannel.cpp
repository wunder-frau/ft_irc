#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "commands/join.hpp"
#include "commands/nick.hpp"
#include "utils.hpp"

// Find a channel by name
Channel* Server::findChannel(const std::string& name) {
    std::string searchName = trimWhitespace(name);
    std::cout << "[DEBUG] findChannel - Looking for '" << name << "'"
              << std::endl;
    std::cout << "[DEBUG] findChannel - After trimming: '" << searchName << "'"
              << std::endl;

    for (size_t i = 0; i < _channels.size(); ++i) {
        std::string channelName = trimWhitespace(_channels[i].getName());
        std::cout << "[DEBUG] findChannel - Comparing with trimmed: '"
                  << channelName << "'" << std::endl;

        if (channelName == searchName) {
            std::cout << "[DEBUG] findChannel - Match found!" << std::endl;
            return &_channels[i];
        }
    }

    std::cout << "[DEBUG] findChannel - No match found" << std::endl;
    return nullptr;
}

// Check if a channel with the given name exists
bool Server::channelExists(const std::string& name) {
    std::string searchName = trimWhitespace(name);

    for (size_t i = 0; i < _channels.size(); ++i) {
        std::string channelName = trimWhitespace(_channels[i].getName());

        if (channelName == searchName) {
            return true;
        }
    }
    return false;
}

void Server::createChannel(const std::string& name, Client* creator) {
    if (!creator)
        return;

    std::string channelName = trimWhitespace(name);
    std::cout << "[DEBUG] Creating new channel with name '" << channelName
              << "'\n";

    Channel newChannel(channelName);
    newChannel.addClient(creator);

    _channels.push_back(newChannel);

    std::cout << "[DEBUG] Channels after creation:\n";
    for (size_t i = 0; i < _channels.size(); ++i) {
        std::cout << "[DEBUG]   " << i << ": '" << _channels[i].getName()
                  << "'\n";
    }
}

// Remove empty channels
void Server::removeEmptyChannels() {
    for (size_t i = 0; i < _channels.size();) {
        if (_channels[i].getClients().empty()) {
            std::cout << "Removing empty channel: " << _channels[i].getName()
                      << std::endl;
            _channels.erase(_channels.begin() + i);
        } else {
            ++i;
        }
    }
}

// Remove a client from all channels
void Server::removeClientFromChannels(int clientFd) {
    Client* client = getClientObjByFd(clientFd);
    if (!client)
        return;

    for (auto& channel : _channels) {
        channel.removeClient(client);
    }

    // Clean up empty channels
    removeEmptyChannels();
}

// JOIN command handler
void Server::handleJoin(int clientFd, const std::string& arg) {
    Client* client = getClientObjByFd(clientFd);
    std::cout << "[DEBUG] handleJoin: fd=" << clientFd << ", arg='" << arg
              << "'\n";

    if (!client || !client->isRegistered()) {
        std::cout
            << "[DEBUG] Client is null or not registered, skipping JOIN.\n";
        return;
    }

    std::vector<std::string> params;
    parser(arg, params, ' ');
    std::cout << "[DEBUG] Parsed JOIN params count = " << params.size() << "\n";

    if (params.size() < 2) {
        std::cout << "[ERROR] Not enough parameters for JOIN command.\n";
        sendError(clientFd, "461", client->getNick(),
                  "JOIN :Not enough parameters");
        return;
    }

    std::vector<std::string> channels;
    parser(params[1], channels, ',');
    std::cout << "[DEBUG] Parsed channels: ";
    for (size_t i = 0; i < channels.size(); ++i)
        std::cout << "'" << channels[i] << "' ";
    std::cout << "\n";

    std::vector<std::string> keys;
    if (params.size() >= 3) {
        parser(params[2], keys, ',');
        std::cout << "[DEBUG] Parsed keys: ";
        for (size_t i = 0; i < keys.size(); ++i)
            std::cout << "'" << keys[i] << "' ";
        std::cout << "\n";
    }

    for (size_t i = 0; i < channels.size(); ++i) {
        std::string channelName = trimWhitespace(channels[i]);
        std::cout << "[DEBUG] Processing channel: '" << channelName << "'\n";

        if (channelName.empty() || channelName[0] != '#') {
            std::cout << "[ERROR] Invalid channel name: '" << channelName
                      << "'\n";
            sendError(clientFd, "403", client->getNick(),
                      channelName + " :No such channel");
            continue;
        }

        std::string key = (i < keys.size()) ? trimWhitespace(keys[i]) : "";
        Channel* channel = findChannel(channelName);

        if (channel) {
            std::cout << "[DEBUG] Channel found: '" << channelName << "'\n";

            if (channel->isInviteOnly() &&
                !channel->isInvited(client->getNick())) {
                std::cout << "[DEBUG] Channel is invite-only and client not "
                             "invited.\n";
                sendError(clientFd, "473", client->getNick(),
                          channelName + " :Cannot join channel (+i)");
                continue;
            }

            if (channel->isInChannel(client)) {
                std::cout << "[DEBUG] Client already in channel, skipping.\n";
                continue;
            }

            if (channel->isKeyed() && key != channel->getKey()) {
                std::cout << "[ERROR] Provided key does not match for channel '"
                          << channelName << "'\n";
                sendError(clientFd, "475", client->getNick(),
                          channelName + " :Cannot join channel (+k)");
                continue;
            }

            int limit = channel->getClientLimit();
            if (limit > -1 &&
                channel->getClients().size() >= static_cast<size_t>(limit)) {
                std::cout << "[ERROR] Channel is full (limit reached): "
                          << limit << "\n";
                sendError(clientFd, "471", client->getNick(),
                          channelName + " :Cannot join channel (+l)");
                continue;
            }

            std::cout << "[DEBUG] Adding client to channel '" << channelName
                      << "'\n";
            channel->addClient(client);

            if (channel->getClients().size() == 1) {
                std::cout
                    << "[DEBUG] First client in channel, assigning operator.\n";
                channel->addOp(clientFd);
            }

            std::string joinMsg = ":" + client->getNick() + "!~" +
                                  client->getUser() + "@" + client->getIPa() +
                                  " JOIN " + channelName + "\r\n";
            channel->broadcast(joinMsg, client);

            if (!channel->getTopic().empty()) {
                std::string topicMsg = ":ft_irc 332 " + client->getNick() +
                                       " " + channelName + " :" +
                                       channel->getTopic() + "\r\n";
                send(clientFd, topicMsg.c_str(), topicMsg.length(), 0);
            }

            std::string namesMsg =
                ":ft_irc 353 " + client->getNick() + " = " + channelName + " :";
            for (const auto& member : channel->getClients()) {
                if (channel->isOperator(member))
                    namesMsg += "@";
                namesMsg += member->getNick() + " ";
            }
            namesMsg += "\r\n";
            send(clientFd, namesMsg.c_str(), namesMsg.length(), 0);

            std::string endNamesMsg = ":ft_irc 366 " + client->getNick() + " " +
                                      channelName + " :End of /NAMES list.\r\n";
            send(clientFd, endNamesMsg.c_str(), endNamesMsg.length(), 0);

            // INFO message
            std::string infoMsg = "[" + channelName + " INFO]: members: ";
            for (const auto& member : channel->getClients())
                infoMsg += member->getNick() + " ";
            infoMsg += "\r\n";
            send(clientFd, infoMsg.c_str(), infoMsg.length(), 0);
        } else {
            std::cout << "[DEBUG] Channel not found. Creating channel: '"
                      << channelName << "'\n";
            createChannel(channelName, client);

            std::string joinMsg = ":" + client->getNick() + "!~" +
                                  client->getUser() + "@" + client->getIPa() +
                                  " JOIN " + channelName + "\r\n";
            send(clientFd, joinMsg.c_str(), joinMsg.length(), 0);

            std::string namesMsg = ":ft_irc 353 " + client->getNick() + " = " +
                                   channelName + " :@" + client->getNick() +
                                   "\r\n";
            send(clientFd, namesMsg.c_str(), namesMsg.length(), 0);

            std::string endNamesMsg = ":ft_irc 366 " + client->getNick() + " " +
                                      channelName + " :End of /NAMES list.\r\n";
            send(clientFd, endNamesMsg.c_str(), endNamesMsg.length(), 0);
            
            std::string infoMsg = "[" + channelName + " INFO]: members: ";
            // Only one member exists (the creator) so far:
            infoMsg += client->getNick() + " ";
            infoMsg += "\r\n";
            send(clientFd, infoMsg.c_str(), infoMsg.length(), 0);
        }
    }
}

// PART command handler
void Server::handlePart(int clientFd, const std::string& arg) {
    Client* client = getClientObjByFd(clientFd);
    if (!client) {
        return;
    }

    // Parse PART command
    std::vector<std::string> params;
    parser(arg, params, ' ');

    if (params.size() < 2) {
        sendError(clientFd, "461", client->getNick(),
                  "PART :Not enough parameters");
        return;
    }

    // Extract channel name(s)
    std::vector<std::string> channels;
    parser(params[1], channels, ',');

    // Extract reason if provided
    std::string reason = "";
    if (params.size() >= 3) {
        // Reconstruct the reason by joining all remaining parameters
        for (size_t i = 2; i < params.size(); ++i) {
            reason += params[i] + " ";
        }
        // Remove leading colon if present
        if (!reason.empty() && reason[0] == ':') {
            reason = reason.substr(1);
        }
    }
    if (reason.empty()) {
        reason = "Leaving";
    }

    // Part each channel
    for (const auto& channel : channels) {
        std::string channelName = trimWhitespace(channel);
        Channel* chan = findChannel(channelName);
        if (!chan) {
            sendError(clientFd, "403", client->getNick(),
                      channelName + " :No such channel");
            continue;
        }

        if (!chan->isInChannel(client)) {
            sendError(clientFd, "442", client->getNick(),
                      channelName + " :You're not on that channel");
            continue;
        }

        // Send PART message to all channel members
        std::string partMsg = ":" + client->getNick() + "!~" +
                              client->getUser() + "@" + client->getIPa() +
                              " PART " + channelName + " :" + reason + "\r\n";
        chan->broadcast(partMsg);

        // Remove client from channel
        chan->removeClient(client);
    }

    // Clean up empty channels
    removeEmptyChannels();
}

// INVITE command handler
void Server::handleInvite(int clientFd, const std::string& arg) {
    Client* sender = getClientObjByFd(clientFd);
    if (!sender) {
        return;
    }

    // Parse INVITE command
    std::vector<std::string> params;
    parser(arg, params, ' ');

    if (params.size() < 3) {
        sendError(clientFd, "461", sender->getNick(),
                  "INVITE :Not enough parameters");
        return;
    }

    std::string targetNick = trimWhitespace(params[1]);
    std::string channelName = trimWhitespace(params[2]);

    // Debug logs to inspect channel name
    std::cout << "[DEBUG] INVITE - Original params[1]: '" << params[1] << "'"
              << std::endl;
    std::cout << "[DEBUG] INVITE - Original params[2]: '" << params[2] << "'"
              << std::endl;
    std::cout << "[DEBUG] INVITE - Target nick after trimming: '" << targetNick
              << "'" << std::endl;
    std::cout << "[DEBUG] INVITE - Channel name after trimming: '"
              << channelName << "'" << std::endl;

    // Debug log to show all channels
    std::cout << "[DEBUG] INVITE - All channels:" << std::endl;
    for (size_t i = 0; i < _channels.size(); ++i) {
        std::string name = _channels[i].getName();
        std::cout << "[DEBUG]   " << i << ": '" << name << "'" << std::endl;
    }

    // Find target client
    Client* target = nullptr;
    for (auto& c : _clients) {
        if (c.getNick() == targetNick) {
            target = &c;
            break;
        }
    }

    if (!target) {
        sendError(clientFd, "401", sender->getNick(),
                  targetNick + " :No such nick/channel");
        return;
    }

    // Find channel
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendError(clientFd, "403", sender->getNick(),
                  channelName + " :No such channel");
        return;
    }

    // Check if sender is in the channel
    if (!channel->isInChannel(sender)) {
        sendError(clientFd, "442", sender->getNick(),
                  channelName + " :You're not on that channel");
        return;
    }

    // Check if sender has permission to invite (must be operator if channel is
    // invite-only)
    if (channel->isInviteOnly() && !channel->isOperator(sender)) {
        sendError(clientFd, "482", sender->getNick(),
                  channelName + " :You're not channel operator");
        return;
    }

    // Check if target is already in the channel
    if (channel->isInChannel(target)) {
        sendError(clientFd, "443", sender->getNick(),
                  targetNick + " " + channelName + " :is already on channel");
        return;
    }

    // Add target to invited list
    channel->addInvited(targetNick);

    // Send invite confirmation to sender
    std::string inviteReply = ":ft_irc 341 " + sender->getNick() + " " +
                              targetNick + " " + channelName + "\r\n";
    send(clientFd, inviteReply.c_str(), inviteReply.length(), 0);

    // Send invite notification to target
    std::string inviteMsg = ":" + sender->getNick() + "!~" + sender->getUser() +
                            "@" + sender->getIPa() + " INVITE " + targetNick +
                            " " + channelName + "\r\n";
    send(target->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);
}

// KICK command handler
void Server::handleKick(int clientFd, const std::string& arg) {
    Client* sender = getClientObjByFd(clientFd);
    if (!sender) {
        return;
    }

    // Parse KICK command
    std::vector<std::string> params;
    parser(arg, params, ' ');

    if (params.size() < 3) {
        sendError(clientFd, "461", sender->getNick(),
                  "KICK :Not enough parameters");
        return;
    }

    std::string channelName = trimWhitespace(params[1]);
    std::string targetNick = trimWhitespace(params[2]);

    // Extract reason if provided
    std::string reason = "";
    if (params.size() >= 4) {
        // Reconstruct the reason by joining all remaining parameters
        for (size_t i = 3; i < params.size(); ++i) {
            reason += params[i] + " ";
        }
        // Remove leading colon if present
        if (!reason.empty() && reason[0] == ':') {
            reason = reason.substr(1);
        }
    }
    if (reason.empty()) {
        reason = targetNick;
    }

    // Find channel
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendError(clientFd, "403", sender->getNick(),
                  channelName + " :No such channel");
        return;
    }

    // Check if sender is in the channel and is an operator
    if (!channel->isInChannel(sender)) {
        sendError(clientFd, "442", sender->getNick(),
                  channelName + " :You're not on that channel");
        return;
    }

    if (!channel->isOperator(sender)) {
        sendError(clientFd, "482", sender->getNick(),
                  channelName + " :You're not channel operator");
        return;
    }

    // Find target client
    Client* target = nullptr;
    for (auto& c : _clients) {
        if (c.getNick() == targetNick) {
            target = &c;
            break;
        }
    }

    if (!target || !channel->isInChannel(target)) {
        sendError(
            clientFd, "441", sender->getNick(),
            targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    // Send KICK message to all channel members
    std::string kickMsg = ":" + sender->getNick() + "!~" + sender->getUser() +
                          "@" + sender->getIPa() + " KICK " + channelName +
                          " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg);

    // Remove target from channel
    channel->removeClient(target);
}

void Server::handleTopic(int clientFd, const std::string& arg) {
    Client* client = getClientObjByFd(clientFd);
    if (!client)
        return;

    // Split off the channel name and the optional new topic
    auto firstSpace = arg.find(' ');
    if (firstSpace == std::string::npos) {
        sendError(clientFd, "461", client->getNick(),
                  "TOPIC :Not enough parameters");
        return;
    }
    std::string remainder = arg.substr(firstSpace + 1);
    // nextSpace separates channel name from topic text
    auto nextSpace = remainder.find(' ');
    std::string channelName = remainder.substr(0, nextSpace);
    std::string rest;
    if (nextSpace != std::string::npos)
        rest = remainder.substr(nextSpace + 1);

    channelName = trimWhitespace(channelName);

    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendError(clientFd, "403", client->getNick(),
                  channelName + " :No such channel");
        return;
    }

    // Must be on the channel
    if (!channel->isInChannel(client)) {
        sendError(clientFd, "442", client->getNick(),
                  channelName + " :You're not on that channel");
        return;
    }

    // GET‐TOPIC: no additional argument => return current topic
    if (rest.empty()) {
        std::string reply;
        if (channel->getTopic().empty()) {
            reply = ":ft_irc 331 " + client->getNick() + " " + channelName +
                    " :No topic is set\r\n";
        } else {
            reply = ":ft_irc 332 " + client->getNick() + " " + channelName +
                    " :" + channel->getTopic() + "\r\n";
        }
        send(clientFd, reply.c_str(), reply.size(), 0);
        return;
    }

    // SET‐TOPIC: if +t is set, only ops can change
    if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        sendError(clientFd, "482", client->getNick(),
                  channelName + " :You're not channel operator");
        return;
    }

    // Strip leading ':' from the new topic per RFC
    if (rest.front() == ':')
        rest.erase(0, 1);

    // Apply the new topic
    channel->setTopic(rest);

    // Broadcast the change
    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                      client->getIPa() + " TOPIC " + channelName + " :" + rest +
                      "\r\n";
    channel->broadcast(msg);
}
