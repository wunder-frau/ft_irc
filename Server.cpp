#include "Server.hpp"

Server::Server(int port, std::string password)
    : _port(port), _password(password) {}

Server::Server(const Server& other)
    : _port(other.getPort()), _password(other.getPassword()) {}

Server::~Server() {}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _port = other.getPort();
        _password = other.getPassword();
    }
    return (*this);
}

// Splits 'arg' by the delimiter 'del' into the vector 'params'
void Server::parser(std::string arg, std::vector<std::string>& params,
                    char del) {
    std::stringstream ss(arg);
    std::string token;
    while (std::getline(ss, token, del)) {
        if (!token.empty())
            params.push_back(token);
    }
}

// Erase client from _clients vector based on file descriptor.
void Server::eraseClient(int clientFd, size_t* clientIndex) {
    (void)clientIndex;
    std::cout << "Erasing client with FD " << clientFd << std::endl;
    
    // Remove client from all channels
    removeClientFromChannels(clientFd);
    
    // Get the client index
    size_t index;
    try {
        index = getClientIndex(clientFd);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }
    
    // Remove client from _clients vector
    _clients.erase(_clients.begin() + index);
    
    // Remove empty channels
    removeEmptyChannels();
}

// Check if the client with file descriptor 'clientFd' is registered.
bool Server::isRegistered(int clientFd) {
    for (std::vector<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        if (it->getFd() == clientFd)
            return it->isRegistered();
    }
    return false;
}

// Check that 'nick' is not already in use by any connected client.
bool Server::isUniqueNick(std::string nick) {
    for (std::vector<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        if (it->getNick() == nick)
            return false;
    }
    return true;
}

// TEST/REMOVE: add a client to the server.
void Server::addClient(const Client& client) { _clients.push_back(client); }

Client* Server::getClientObjByFd(int fd) {
    for (auto& client : _clients) {
        if (client.getFd() == fd)
            return &client;
    }
    return nullptr;
}

// Channel-related methods
Channel* Server::findChannel(const std::string& name) {
    for (auto& channel : _channels) {
        if (channel.getName() == name)
            return &channel;
    }
    return nullptr;
}

bool Server::channelExists(const std::string& name) {
    return findChannel(name) != nullptr;
}

void Server::createChannel(const std::string& name, Client* creator) {
    Channel newChannel(name);
    newChannel.addClient(creator);
    _channels.push_back(newChannel);
}

void Server::removeEmptyChannels() {
    for (auto it = _channels.begin(); it != _channels.end();) {
        if (it->getClients().empty()) {
            it = _channels.erase(it);
        } else {
            ++it;
        }
    }
}

void Server::removeClientFromChannels(int clientFd) {
    Client* client = getClientObjByFd(clientFd);
    if (!client) return;
    
    for (auto& channel : _channels) {
        channel.removeClient(client);
    }
}

// Channel commands
void Server::handleJoin(int clientFd, const std::string& arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 2) {
        sendError(clientFd, "461", _clients[getClientIndex(clientFd)].getNick(), "JOIN :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    
    // Validate channel name
    if (channelName[0] != '#') {
        sendError(clientFd, "403", _clients[getClientIndex(clientFd)].getNick(), channelName + " :No such channel");
        return;
    }
    
    Channel* channel = findChannel(channelName);
    Client* client = getClientObjByFd(clientFd);
    
    if (!client) {
        std::cerr << "Error: Client not found for FD " << clientFd << std::endl;
        return;
    }
    
    if (!channel) {
        // Create new channel
        createChannel(channelName, client);
        std::string joinMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIPa() + " JOIN " + channelName + "\r\n";
        send(clientFd, joinMsg.c_str(), joinMsg.length(), 0);
    } else {
        // Join existing channel
        // Check if invite-only
        if (channel->isInviteOnly() && !channel->isInvited(client->getNick())) {
            sendError(clientFd, "473", client->getNick(), channelName + " :Cannot join channel (+i)");
            return;
        }
        
        // Add client to channel
        channel->addClient(client);
        
        // Send join confirmation
        std::string joinMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIPa() + " JOIN " + channelName + "\r\n";
        channel->broadcast(joinMsg);
        
        // Send channel topic
        if (!channel->getTopic().empty()) {
            std::string topicMsg = ":server 332 " + client->getNick() + " " + channelName + " :" + channel->getTopic() + "\r\n";
            send(clientFd, topicMsg.c_str(), topicMsg.length(), 0);
        }
    }
}

void Server::handlePart(int clientFd, const std::string& arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 2) {
        sendError(clientFd, "461", _clients[getClientIndex(clientFd)].getNick(), "PART :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    Channel* channel = findChannel(channelName);
    Client* client = getClientObjByFd(clientFd);
    
    if (!client) {
        std::cerr << "Error: Client not found for FD " << clientFd << std::endl;
        return;
    }
    
    if (!channel) {
        sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
        return;
    }
    
    if (!channel->isInChannel(client)) {
        sendError(clientFd, "442", client->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    // Create part message
    std::string partReason = (params.size() > 2) ? params[2] : "Leaving";
    std::string partMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIPa() + " PART " + channelName + " :" + partReason + "\r\n";
    channel->broadcast(partMsg);
    
    // Remove client from channel
    channel->removeClient(client);
}

void Server::handleInvite(int clientFd, const std::string& arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 3) {
        sendError(clientFd, "461", _clients[getClientIndex(clientFd)].getNick(), "INVITE :Not enough parameters");
        return;
    }
    
    std::string targetNick = params[1];
    std::string channelName = params[2];
    
    Channel* channel = findChannel(channelName);
    Client* sender = getClientObjByFd(clientFd);
    
    if (!sender) {
        std::cerr << "Error: Client not found for FD " << clientFd << std::endl;
        return;
    }
    
    if (!channel) {
        sendError(clientFd, "403", sender->getNick(), channelName + " :No such channel");
        return;
    }
    
    // Check if sender is in the channel
    if (!channel->isInChannel(sender)) {
        sendError(clientFd, "442", sender->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    // Check if sender is operator (only needed for invite-only channels)
    if (channel->isInviteOnly() && !channel->isOperator(sender)) {
        sendError(clientFd, "482", sender->getNick(), channelName + " :You're not channel operator");
        return;
    }
    
    // Find target client
    Client* target = nullptr;
    for (auto& client : _clients) {
        if (client.getNick() == targetNick) {
            target = &client;
            break;
        }
    }
    
    if (!target) {
        sendError(clientFd, "401", sender->getNick(), targetNick + " :No such nick");
        return;
    }
    
    // Check if target is already in the channel
    if (channel->isInChannel(target)) {
        sendError(clientFd, "443", sender->getNick(), targetNick + " " + channelName + " :is already on channel");
        return;
    }
    
    // Add target to invited list
    channel->addInvited(targetNick);
    
    // Send invite notification to target
    std::string inviteMsg = ":" + sender->getNick() + "!" + sender->getUser() + "@" + sender->getIPa() + " INVITE " + targetNick + " " + channelName + "\r\n";
    send(target->getFd(), inviteMsg.c_str(), inviteMsg.length(), 0);
    
    // Send confirmation to sender
    std::string confirmMsg = ":server 341 " + sender->getNick() + " " + targetNick + " " + channelName + "\r\n";
    send(sender->getFd(), confirmMsg.c_str(), confirmMsg.length(), 0);
}

void Server::handleKick(int clientFd, const std::string& arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 3) {
        sendError(clientFd, "461", _clients[getClientIndex(clientFd)].getNick(), "KICK :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    std::string targetNick = params[2];
    std::string kickReason = (params.size() > 3) ? params[3] : "No reason given";
    
    Channel* channel = findChannel(channelName);
    Client* sender = getClientObjByFd(clientFd);
    
    if (!sender) {
        std::cerr << "Error: Client not found for FD " << clientFd << std::endl;
        return;
    }
    
    if (!channel) {
        sendError(clientFd, "403", sender->getNick(), channelName + " :No such channel");
        return;
    }
    
    // Check if sender is in the channel
    if (!channel->isInChannel(sender)) {
        sendError(clientFd, "442", sender->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    // Check if sender is operator
    if (!channel->isOperator(sender)) {
        sendError(clientFd, "482", sender->getNick(), channelName + " :You're not channel operator");
        return;
    }
    
    // Find target client
    Client* target = nullptr;
    for (auto& client : _clients) {
        if (client.getNick() == targetNick) {
            target = &client;
            break;
        }
    }
    
    if (!target) {
        sendError(clientFd, "401", sender->getNick(), targetNick + " :No such nick");
        return;
    }
    
    // Check if target is in the channel
    if (!channel->isInChannel(target)) {
        sendError(clientFd, "441", sender->getNick(), targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }
    
    // Send kick message to all users in the channel
    std::string kickMsg = ":" + sender->getNick() + "!" + sender->getUser() + "@" + sender->getIPa() + " KICK " + channelName + " " + targetNick + " :" + kickReason + "\r\n";
    channel->broadcast(kickMsg);
    
    // Remove the target from the channel
    channel->removeClient(target);
}

void Server::handleTopic(int clientFd, const std::string& arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 2) {
        sendError(clientFd, "461", _clients[getClientIndex(clientFd)].getNick(), "TOPIC :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    Channel* channel = findChannel(channelName);
    Client* client = getClientObjByFd(clientFd);
    
    if (!client) {
        std::cerr << "Error: Client not found for FD " << clientFd << std::endl;
        return;
    }
    
    if (!channel) {
        sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
        return;
    }
    
    // Check if client is in the channel
    if (!channel->isInChannel(client)) {
        sendError(clientFd, "442", client->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    // If no topic is provided, send the current topic
    if (params.size() == 2) {
        std::string topic = channel->getTopic();
        if (topic.empty()) {
            std::string noTopicMsg = ":server 331 " + client->getNick() + " " + channelName + " :No topic is set\r\n";
            send(clientFd, noTopicMsg.c_str(), noTopicMsg.length(), 0);
        } else {
            std::string topicMsg = ":server 332 " + client->getNick() + " " + channelName + " :" + topic + "\r\n";
            send(clientFd, topicMsg.c_str(), topicMsg.length(), 0);
        }
        return;
    }
    
    // Setting a new topic requires operator privileges
    if (!channel->isOperator(client)) {
        sendError(clientFd, "482", client->getNick(), channelName + " :You're not channel operator");
        return;
    }
    
    // Set the new topic
    std::string newTopic = params[2];
    channel->setTopic(newTopic);
    
    // Broadcast topic change to all users in the channel
    std::string topicMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIPa() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel->broadcast(topicMsg);
}

void Server::handleMode(int clientFd, const std::string& arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');
    
    if (params.size() < 3) {
        sendError(clientFd, "461", _clients[getClientIndex(clientFd)].getNick(), "MODE :Not enough parameters");
        return;
    }
    
    std::string channelName = params[1];
    std::string modeString = params[2];
    
    Channel* channel = findChannel(channelName);
    Client* client = getClientObjByFd(clientFd);
    
    if (!client) {
        std::cerr << "Error: Client not found for FD " << clientFd << std::endl;
        return;
    }
    
    if (!channel) {
        sendError(clientFd, "403", client->getNick(), channelName + " :No such channel");
        return;
    }
    
    // Check if client is in the channel
    if (!channel->isInChannel(client)) {
        sendError(clientFd, "442", client->getNick(), channelName + " :You're not on that channel");
        return;
    }
    
    // Check if client is operator
    if (!channel->isOperator(client)) {
        sendError(clientFd, "482", client->getNick(), channelName + " :You're not channel operator");
        return;
    }
    
    // Handle mode +i (set invite-only)
    if (modeString == "+i") {
        channel->setInviteOnly(true);
        std::string modeMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIPa() + " MODE " + channelName + " +i\r\n";
        channel->broadcast(modeMsg);
    }
    // Handle mode -i (remove invite-only)
    else if (modeString == "-i") {
        channel->setInviteOnly(false);
        std::string modeMsg = ":" + client->getNick() + "!" + client->getUser() + "@" + client->getIPa() + " MODE " + channelName + " -i\r\n";
        channel->broadcast(modeMsg);
    }
    // Other modes not supported
    else {
        sendError(clientFd, "472", client->getNick(), modeString + " :is unknown mode char to me");
    }
}
