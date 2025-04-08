#include "Server.hpp"

// Find a channel by name
Channel* Server::findChannel(const std::string& name) {
    for (size_t i = 0; i < _channels.size(); ++i) {
        if (_channels[i].getName() == name) {
            return &_channels[i];
        }
    }
    return nullptr;
}

// Check if a channel with the given name exists
bool Server::channelExists(const std::string& name) {
    return findChannel(name) != nullptr;
}

// Create a new channel with the given name and creator
void Server::createChannel(const std::string& name, Client* creator) {
    if (creator == nullptr)
        return;
        
    Channel newChannel(name);
    _channels.push_back(newChannel);
    
    // Get the newly created channel and add the creator
    Channel* channel = findChannel(name);
    if (channel) {
        channel->addClient(creator);
    }
}

// Remove empty channels
void Server::removeEmptyChannels() {
    for (size_t i = 0; i < _channels.size();) {
        if (_channels[i].getClients().empty()) {
            std::cout << "Removing empty channel: " << _channels[i].getName() << std::endl;
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
    if (!client) {
        return;
    }
    
    // Parse JOIN command
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 2) {
        sendError(clientFd, "461", client->getNick(), "JOIN :Not enough parameters");
        return;
    }
    
    // Extract channel name(s)
    std::vector<std::string> channels;
    parser(params[1], channels, ',');
    
    // Extract keys if provided
    std::vector<std::string> keys;
    if (params.size() >= 3) {
        parser(params[2], keys, ',');
    }
    
    // Join each channel
    for (size_t i = 0; i < channels.size(); ++i) {
        std::string channelName = channels[i];
        
        // Validate channel name format (should start with #)
        if (channelName.empty() || channelName[0] != '#') {
            sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
            continue;
        }
        
        // Get key if provided
        std::string key = (i < keys.size()) ? keys[i] : "";
        
        // Check if channel exists
        Channel* channel = findChannel(channelName);
        if (channel) {
            // Channel exists, check if client can join
            if (channel->isInviteOnly() && !channel->isInvited(client->getNick())) {
                sendError(clientFd, "473", client->getNick(), channelName + " :Cannot join channel (+i)");
                continue;
            }
            
            // If client is already in the channel, do nothing
            if (channel->isInChannel(client)) {
                continue;
            }
            
            // Add client to channel
            channel->addClient(client);
            
            // Send JOIN message to all channel members
            std::string joinMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" + 
                                  client->getIPa() + " JOIN " + channelName + "\r\n";
            channel->broadcast(joinMsg);
            
            // Send topic if available
            if (!channel->getTopic().empty()) {
                std::string topicMsg = ":ft_irc 332 " + client->getNick() + " " + 
                                       channelName + " :" + channel->getTopic() + "\r\n";
                send(clientFd, topicMsg.c_str(), topicMsg.length(), 0);
            }
            
            // Send user list
            std::string namesMsg = ":ft_irc 353 " + client->getNick() + " = " + 
                                   channelName + " :";
            for (const auto& member : channel->getClients()) {
                if (channel->isOperator(member)) {
                    namesMsg += "@";
                }
                namesMsg += member->getNick() + " ";
            }
            namesMsg += "\r\n";
            send(clientFd, namesMsg.c_str(), namesMsg.length(), 0);
            
            // End of names list
            std::string endNamesMsg = ":ft_irc 366 " + client->getNick() + " " + 
                                      channelName + " :End of /NAMES list.\r\n";
            send(clientFd, endNamesMsg.c_str(), endNamesMsg.length(), 0);
        } else {
            // Channel doesn't exist, create it
            createChannel(channelName, client);
            
            // Send JOIN message to the client
            std::string joinMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" + 
                                  client->getIPa() + " JOIN " + channelName + "\r\n";
            send(clientFd, joinMsg.c_str(), joinMsg.length(), 0);
            
            // Send initial channel info
            std::string namesMsg = ":ft_irc 353 " + client->getNick() + " = " + 
                                   channelName + " :@" + client->getNick() + "\r\n";
            send(clientFd, namesMsg.c_str(), namesMsg.length(), 0);
            
            std::string endNamesMsg = ":ft_irc 366 " + client->getNick() + " " + 
                                      channelName + " :End of /NAMES list.\r\n";
            send(clientFd, endNamesMsg.c_str(), endNamesMsg.length(), 0);
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
        sendError(clientFd, "461", client->getNick(), "PART :Not enough parameters");
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
    for (const auto& channelName : channels) {
        Channel* channel = findChannel(channelName);
        if (!channel) {
            sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
            continue;
        }
        
        if (!channel->isInChannel(client)) {
            sendError(clientFd, "442", client->getNick(), channelName + " :You're not on that channel");
            continue;
        }
        
        // Send PART message to all channel members
        std::string partMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" + 
                              client->getIPa() + " PART " + channelName + " :" + reason + "\r\n";
        channel->broadcast(partMsg);
        
        // Remove client from channel
        channel->removeClient(client);
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
        sendError(clientFd, "461", sender->getNick(), "INVITE :Not enough parameters");
        return;
    }
    
    std::string targetNick = params[1];
    std::string channelName = params[2];
    
    // Find target client
    Client* target = nullptr;
    for (auto& c : _clients) {
        if (c.getNick() == targetNick) {
            target = &c;
            break;
        }
    }
    
    if (!target) {
        sendError(clientFd, "401", sender->getNick(), targetNick + " :No such nick/channel");
        return;
    }
    
    // Find channel
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendError(clientFd, "403", sender->getNick(), channelName + " :No such channel");
        return;
    }
    
    // Check if sender is in the channel
    if (!channel->isInChannel(sender)) {
        sendError(clientFd, "442", sender->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    // Check if sender has permission to invite (must be operator if channel is invite-only)
    if (channel->isInviteOnly() && !channel->isOperator(sender)) {
        sendError(clientFd, "482", sender->getNick(), channelName + " :You're not channel operator");
        return;
    }
    
    // Check if target is already in the channel
    if (channel->isInChannel(target)) {
        sendError(clientFd, "443", sender->getNick(), targetNick + " " + channelName + " :is already on channel");
        return;
    }
    
    // Add target to invited list
    channel->addInvited(targetNick);
    
    // Send invite confirmation to sender
    std::string inviteReply = ":ft_irc 341 " + sender->getNick() + " " + targetNick + " " + channelName + "\r\n";
    send(clientFd, inviteReply.c_str(), inviteReply.length(), 0);
    
    // Send invite notification to target
    std::string inviteMsg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" + 
                           sender->getIPa() + " INVITE " + targetNick + " " + channelName + "\r\n";
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
        sendError(clientFd, "461", sender->getNick(), "KICK :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    std::string targetNick = params[2];
    
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
        sendError(clientFd, "403", sender->getNick(), channelName + " :No such channel");
        return;
    }
    
    // Check if sender is in the channel and is an operator
    if (!channel->isInChannel(sender)) {
        sendError(clientFd, "442", sender->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    if (!channel->isOperator(sender)) {
        sendError(clientFd, "482", sender->getNick(), channelName + " :You're not channel operator");
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
        sendError(clientFd, "441", sender->getNick(), targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }
    
    // Send KICK message to all channel members
    std::string kickMsg = ":" + sender->getNick() + "!~" + sender->getUser() + "@" + 
                         sender->getIPa() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg);
    
    // Remove target from channel
    channel->removeClient(target);
}

// TOPIC command handler
void Server::handleTopic(int clientFd, const std::string& arg) {
    Client* client = getClientObjByFd(clientFd);
    if (!client) {
        return;
    }
    
    // Parse TOPIC command
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 2) {
        sendError(clientFd, "461", client->getNick(), "TOPIC :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    
    // Find channel
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
        return;
    }
    
    // Check if client is in the channel
    if (!channel->isInChannel(client)) {
        sendError(clientFd, "442", client->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    // If no topic is provided, return the current topic
    if (params.size() == 2) {
        if (channel->getTopic().empty()) {
            std::string noTopicMsg = ":ft_irc 331 " + client->getNick() + " " + channelName + " :No topic is set\r\n";
            send(clientFd, noTopicMsg.c_str(), noTopicMsg.length(), 0);
        } else {
            std::string topicMsg = ":ft_irc 332 " + client->getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n";
            send(clientFd, topicMsg.c_str(), topicMsg.length(), 0);
        }
        return;
    }
    
    // Setting a new topic requires operator privileges
    if (!channel->isOperator(client)) {
        sendError(clientFd, "482", client->getNick(), channelName + " :You're not channel operator");
        return;
    }
    
    // Extract new topic (combine all remaining parameters)
    std::string newTopic = "";
    for (size_t i = 2; i < params.size(); ++i) {
        newTopic += params[i] + " ";
    }
    // Remove leading colon if present
    if (!newTopic.empty() && newTopic[0] == ':') {
        newTopic = newTopic.substr(1);
    }
    
    // Set the new topic
    channel->setTopic(newTopic);
    
    // Broadcast topic change to all channel members
    std::string topicChangeMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" + 
                               client->getIPa() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel->broadcast(topicChangeMsg);
}

// MODE command handler
void Server::handleMode(int clientFd, const std::string& arg) {
    Client* client = getClientObjByFd(clientFd);
    if (!client) {
        return;
    }
    
    // Parse MODE command
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 2) {
        sendError(clientFd, "461", client->getNick(), "MODE :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    
    // Find channel
    Channel* channel = findChannel(channelName);
    if (!channel) {
        sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
        return;
    }
    
    // If no mode is provided, return current modes
    if (params.size() == 2) {
        std::string modeMsg = ":ft_irc 324 " + client->getNick() + " " + channelName + " +";
        if (channel->isInviteOnly()) {
            modeMsg += "i";
        }
        modeMsg += "\r\n";
        send(clientFd, modeMsg.c_str(), modeMsg.length(), 0);
        return;
    }
    
    // Check if client is in the channel and is an operator
    if (!channel->isInChannel(client)) {
        sendError(clientFd, "442", client->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    if (!channel->isOperator(client)) {
        sendError(clientFd, "482", client->getNick(), channelName + " :You're not channel operator");
        return;
    }
    
    // Parse mode string
    std::string modeString = params[2];
    if (modeString.empty() || (modeString[0] != '+' && modeString[0] != '-')) {
        sendError(clientFd, "472", client->getNick(), modeString + " :is unknown mode char to me");
        return;
    }
    
    bool adding = (modeString[0] == '+');
    
    // Process each mode character
    for (size_t i = 1; i < modeString.length(); ++i) {
        char modeChar = modeString[i];
        std::string modeChangeMsg;  // Define the string outside the switch
        
        // Handle different modes
        switch (modeChar) {
            case 'i': // Invite-only
                channel->setInviteOnly(adding);
                
                // Broadcast mode change
                modeChangeMsg = ":" + client->getNick() + "!~" + client->getUser() + "@" + 
                               client->getIPa() + " MODE " + channelName + " " + 
                               (adding ? "+i" : "-i") + "\r\n";
                channel->broadcast(modeChangeMsg);
                break;
                
            // Add other mode characters here as needed
            
            default:
                sendError(clientFd, "472", client->getNick(), std::string(1, modeChar) + " :is unknown mode char to me");
                break;
        }
    }
} 