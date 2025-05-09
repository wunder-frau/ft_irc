#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "commands/join.hpp"
#include "commands/nick.hpp"
#include "utils.hpp"

// Find a channel by name
Channel* Server::findChannel(const std::string& name)
{
    std::string searchKey = ircCaseFold(trimWhitespace(name));
    std::cout << "[findChannel] searchKey: '" << searchKey << "'\n";
    debugLog("findChannel - Looking for '" + name + "'");
    debugLog("findChannel - After trimming: '" + searchKey + "'");

    for (size_t i = 0; i < _channels.size(); ++i)
    {
        std::string existingKey = ircCaseFold(_channels[i].getName());

        std::cout << "[findChannel] checking: '" << existingKey << "'\n";
        debugLog("Comparing with channel[" + std::to_string(i) + "] = '" + existingKey + "'");

        if (existingKey == searchKey)
        {
            debugLog("findChannel - Match found!");
            std::cout << "[findChannel] found match!\n";
            return &_channels[i];
        }
    }
    return nullptr;
}

// Check if a channel with the given name exists
bool Server::channelExists(const std::string& name)
{
    std::string searchName = trimWhitespace(name);

    for (size_t i = 0; i < _channels.size(); ++i)
    {
        std::string channelName = trimWhitespace(_channels[i].getName());

        if (channelName == searchName)
        {
            return true;
        }
    }
    return false;
}

void Server::createChannel(const std::string& name, Client* creator)
{
    if (!creator)
        return;

    std::string channelName = trimWhitespace(name);
    std::string channelKey = normalizeChannelName(channelName);
    debugLog("Creating new channel with name '" + channelName + "'");

    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (normalizeChannelName(_channels[i].getName()) == channelKey)
        {
            debugLog("Channel '" + channelKey + "' already exists (as '" + _channels[i].getName() +
                     "')");
            _channels[i].addClient(creator);
            return;
        }
    }
    Channel newChannel(channelName);
    newChannel.addClient(creator);

    _channels.push_back(newChannel);

    debugLog("Channels after creation:");
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        debugLog("  " + std::to_string(i) + ": '" + _channels[i].getName() + "'");
    }
}

// Remove empty channels
void Server::removeEmptyChannels()
{
    for (size_t i = 0; i < _channels.size();)
    {
        if (_channels[i].getClients().empty())
        {
            // Replace std::cout with debugLog
            debugLog("Removing empty channel: " + _channels[i].getName());
            _channels.erase(_channels.begin() + i);
        }
        else
        {
            ++i;
        }
    }
}

// Remove a client from all channels
void Server::removeClientFromChannels(int clientFd)
{
    Client* client = getClientObjByFd(clientFd);
    if (!client)
        return;

    for (auto& channel : _channels)
    {
        channel.removeClient(client);
    }

    // Clean up empty channels
    removeEmptyChannels();
}

// JOIN command handler
void Server::handleJoin(int clientFd, const std::string& arg)
{
    Client* client = getClientObjByFd(clientFd);
    debugLog("handleJoin: fd=" + std::to_string(clientFd) + ", arg='" + arg + "'");

    if (!client || !client->isRegistered())
    {
        debugLog("Client is null or not registered, skipping JOIN.");
        return;
    }

    std::vector<std::string> params;
    parser(arg, params, ' ');
    debugLog("Parsed JOIN params count = " + std::to_string(params.size()));

    if (params.size() < 2)
    {
        std::cout << "[ERROR] Not enough parameters for JOIN command.\n";
        sendError(clientFd, "461", client->getNick(), "JOIN :Not enough parameters");
        return;
    }

    std::vector<std::string> channels;
    parser(params[1], channels, ',');
    std::string channelsDebug = "Parsed channels: ";
    for (size_t i = 0; i < channels.size(); ++i) channelsDebug += "'" + channels[i] + "' ";
    debugLog(channelsDebug);

    std::vector<std::string> keys;
    if (params.size() >= 3)
    {
        parser(params[2], keys, ',');
        std::string keysDebug = "Parsed keys: ";
        for (size_t i = 0; i < keys.size(); ++i) keysDebug += "'" + keys[i] + "' ";
        debugLog(keysDebug);
    }

    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string channelName = trimWhitespace(channels[i]);
        debugLog("Processing channel: '" + channelName + "'");
        if (channelName.length() > 50)
        {
            debugLog("Channel name too long: '" + channelName + "'");
            sendError(clientFd, "479", client->getNick(),
                      channelName + " :Channel name is too long (max 50 characters)");
            continue;
        }
        if (channelName.empty() || channelName[0] != '#')
        {
            std::cout << "[ERROR] Invalid channel name: '" << channelName << "'\n";
            sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
            continue;
        }

        std::string key = (i < keys.size()) ? trimWhitespace(keys[i]) : "";
        Channel* channel = findChannel(channelName);

        if (channel)
        {
            debugLog("Channel found: '" + channelName + "'");

            if (channel->isInviteOnly() && !channel->isInvited(client->getNick()))
            {
                debugLog("Channel is invite-only and client not invited.");
                sendError(clientFd, "473", client->getNick(),
                          channelName + " :Cannot join channel (+i)");
                continue;
            }

            if (channel->isInChannel(client))
            {
                debugLog("Client already in channel, skipping.");
                continue;
            }

            if (channel->isKeyed() && key != channel->getModeKey())
            {
                std::cout << "[ERROR] Provided key does not match for channel '" << channelName
                          << "'\n";
                sendError(clientFd, "475", client->getNick(),
                          channelName + " :Cannot join channel (+k)");
                continue;
            }

            int limit = channel->getClientLimit();
            if (limit > -1 && channel->getClients().size() >= static_cast<size_t>(limit))
            {
                std::cout << "[ERROR] Channel is full (limit reached): " << limit << "\n";
                sendError(clientFd, "471", client->getNick(),
                          channelName + " :Cannot join channel (+l)");
                continue;
            }

            debugLog("Adding client to channel '" + channelName + "'");
            channel->addClient(client);

            if (channel->getClients().size() == 1)
            {
                debugLog("First client in channel, assigning operator.");
                channel->addOp(clientFd);
            }

            std::string joinMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                                  client->getIPa() + " JOIN " + channelName + "\r\n";
            channel->broadcast(joinMsg, client);

            if (!channel->getTopic().empty())
            {
                std::string topicMsg = ":ft_irc 332 " + client->getNick() + " " + channelName +
                                       " :" + channel->getTopic() + "\r\n";
                send(clientFd, topicMsg.c_str(), topicMsg.length(), 0);
            }

            std::string namesMsg = ":ft_irc 353 " + client->getNick() + " = " + channelName + " :";
            for (const auto& member : channel->getClients())
            {
                if (channel->isOperator(member))
                    namesMsg += "@";
                namesMsg += member->getNick() + " ";
            }
            namesMsg += "\r\n";
            send(clientFd, namesMsg.c_str(), namesMsg.length(), 0);

            std::string endNamesMsg = ":ft_irc 366 " + client->getNick() + " " + channelName +
                                      " :End of /NAMES list.\r\n";
            send(clientFd, endNamesMsg.c_str(), endNamesMsg.length(), 0);

            // INFO message
            std::string infoMsg = "[" + channelName + " INFO]: members: ";
            for (const auto& member : channel->getClients()) infoMsg += member->getNick() + " ";
            infoMsg += "\r\n";
            send(clientFd, infoMsg.c_str(), infoMsg.length(), 0);
        }
        else
        {
            debugLog("Channel not found. Creating channel: '" + channelName + "'");
            createChannel(channelName, client);

            std::string joinMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                                  client->getIPa() + " JOIN " + channelName + "\r\n";
            send(clientFd, joinMsg.c_str(), joinMsg.length(), 0);

            std::string namesMsg = ":ft_irc 353 " + client->getNick() + " = " + channelName +
                                   " :@" + client->getNick() + "\r\n";
            send(clientFd, namesMsg.c_str(), namesMsg.length(), 0);

            std::string endNamesMsg = ":ft_irc 366 " + client->getNick() + " " + channelName +
                                      " :End of /NAMES list.\r\n";
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
void Server::handlePart(int clientFd, const std::string& arg)
{
    Client* client = getClientObjByFd(clientFd);
    if (!client)
    {
        return;
    }

    // Parse PART command
    std::vector<std::string> params;
    parser(arg, params, ' ');

    if (params.size() < 2)
    {
        sendError(clientFd, "461", client->getNick(), "PART :Not enough parameters");
        return;
    }

    // Extract channel name(s)
    std::vector<std::string> channels;
    parser(params[1], channels, ',');

    // Extract reason if provided
    std::string reason = "";
    if (params.size() >= 3)
    {
        // Reconstruct the reason by joining all remaining parameters
        for (size_t i = 2; i < params.size(); ++i)
        {
            reason += params[i] + " ";
        }
        // Remove leading colon if present
        if (!reason.empty() && reason[0] == ':')
        {
            reason = reason.substr(1);
        }
    }
    if (reason.empty())
    {
        reason = "Leaving";
    }

    // Part each channel
    for (const auto& channel : channels)
    {
        std::string channelName = trimWhitespace(channel);
        Channel* chan = findChannel(channelName);
        if (!chan)
        {
            sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
            continue;
        }

        if (!chan->isInChannel(client))
        {
            sendError(clientFd, "442", client->getNick(),
                      channelName + " :You're not on that channel");
            continue;
        }

        // Send PART message to all channel members
        std::string partMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                              client->getIPa() + " PART " + channelName + " :" + reason + "\r\n";
        chan->broadcast(partMsg);

        // Remove client from channel
        chan->removeClient(client);
    }

    // Clean up empty channels
    removeEmptyChannels();
}

// INVITE command handler
void Server::handleInvite(int clientFd, const std::string& arg)
{
    Client* sender = getClientObjByFd(clientFd);
    if (!sender)
    {
        return;
    }

    // Parse INVITE command
    std::vector<std::string> params;
    parser(arg, params, ' ');

    if (params.size() < 3)
    {
        sendError(clientFd, "461", sender->getNick(), "INVITE :Not enough parameters");
        return;
    }

    std::string targetNick = trimWhitespace(params[1]);
    std::string channelName = trimWhitespace(params[2]);

    // Debug log to show all channels
    debugLog("INVITE - All channels:");
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        std::string name = _channels[i].getName();
        debugLog("  " + std::to_string(i) + ": '" + name + "'");
    }

    // Find target client
    Client* target = nullptr;
    for (auto& c : _clients)
    {
        if (c.getNick() == targetNick)
        {
            target = &c;
            break;
        }
    }

    if (!target)
    {
        sendError(clientFd, "401", sender->getNick(), targetNick + " :No such nick/channel");
        return;
    }

    // Find channel
    Channel* channel = findChannel(channelName);
    if (!channel)
    {
        sendError(clientFd, "403", sender->getNick(), channelName + " :No such channel");
        return;
    }

    // Check if sender is in the channel
    if (!channel->isInChannel(sender))
    {
        sendError(clientFd, "442", sender->getNick(), channelName + " :You're not on that channel");
        return;
    }

    // Check if sender has permission to invite (must be operator if channel is
    // invite-only)
    if (channel->isInviteOnly() && !channel->isOperator(sender))
    {
        sendError(clientFd, "482", sender->getNick(),
                  channelName + " :You're not channel operator");
        return;
    }

    // Check if target is already in the channel
    if (channel->isInChannel(target))
    {
        sendError(clientFd, "443", sender->getNick(),
                  targetNick + " " + channelName + " :is already on channel");
        return;
    }

    // Add target to invited list
    channel->addInvited(targetNick);

    // Send invite confirmation to sender
    std::string inviteReply =
        ":ft_irc 341 " + sender->getNick() + " " + targetNick + " " + channelName + "\r\n";
    send(clientFd, inviteReply.c_str(), inviteReply.length(), 0);

    // Send invite notification to target
    std::string inviteMsg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" +
                            sender->getIPa() + " INVITE " + targetNick + " " + channelName + "\r\n";
    send(target->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);
}

// KICK command handler
void Server::handleKick(int clientFd, const std::string& arg)
{
    Client* sender = getClientObjByFd(clientFd);
    if (!sender)
    {
        return;
    }

    // Parse KICK command
    std::vector<std::string> params;
    parser(arg, params, ' ');

    if (params.size() < 3)
    {
        sendError(clientFd, "461", sender->getNick(), "KICK :Not enough parameters");
        return;
    }

    std::string channelName = trimWhitespace(params[1]);
    std::string targetNick = trimWhitespace(params[2]);

    // Extract reason if provided
    std::string reason = "";
    if (params.size() >= 4)
    {
        // Reconstruct the reason by joining all remaining parameters
        for (size_t i = 3; i < params.size(); ++i)
        {
            reason += params[i] + " ";
        }
        // Remove leading colon if present
        if (!reason.empty() && reason[0] == ':')
        {
            reason = reason.substr(1);
        }
    }
    if (reason.empty())
    {
        reason = targetNick;
    }

    // Find channel
    Channel* channel = findChannel(channelName);
    if (!channel)
    {
        sendError(clientFd, "403", sender->getNick(), channelName + " :No such channel");
        return;
    }

    // Check if sender is in the channel and is an operator
    if (!channel->isInChannel(sender))
    {
        sendError(clientFd, "442", sender->getNick(), channelName + " :You're not on that channel");
        return;
    }

    if (!channel->isOperator(sender))
    {
        sendError(clientFd, "482", sender->getNick(),
                  channelName + " :You're not channel operator");
        return;
    }

    // Find target client
    Client* target = nullptr;
    for (auto& c : _clients)
    {
        if (c.getNick() == targetNick)
        {
            target = &c;
            break;
        }
    }

    if (!target || !channel->isInChannel(target))
    {
        sendError(clientFd, "441", sender->getNick(),
                  targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    // Send KICK message to all channel members
    std::string kickMsg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" +
                          sender->getIPa() + " KICK " + channelName + " " + targetNick + " :" +
                          reason + "\r\n";
    channel->broadcast(kickMsg);

    // Remove target from channel
    channel->removeClient(target);
}

void Server::handleTopic(int clientFd, const std::string& arg)
{
    Client* client = getClientObjByFd(clientFd);
    if (!client)
        return;

    // Split off the channel name and the optional new topic
    auto firstSpace = arg.find(' ');
    if (firstSpace == std::string::npos)
    {
        sendError(clientFd, "461", client->getNick(), "TOPIC :Not enough parameters");
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
    if (!channel)
    {
        sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
        return;
    }

    // Must be on the channel
    if (!channel->isInChannel(client))
    {
        sendError(clientFd, "442", client->getNick(), channelName + " :You're not on that channel");
        return;
    }

    // GET‐TOPIC: no additional argument => return current topic
    if (rest.empty())
    {
        std::string reply;
        if (channel->getTopic().empty())
        {
            reply =
                ":ft_irc 331 " + client->getNick() + " " + channelName + " :No topic is set\r\n";
        }
        else
        {
            reply = ":ft_irc 332 " + client->getNick() + " " + channelName + " :" +
                    channel->getTopic() + "\r\n";
        }
        send(clientFd, reply.c_str(), reply.size(), 0);
        return;
    }

    // SET‐TOPIC: if +t is set, only ops can change
    if (channel->isTopicRestricted() && !channel->isOperator(client))
    {
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
    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" + client->getIPa() +
                      " TOPIC " + channelName + " :" + rest + "\r\n";
    channel->broadcast(msg);
}
