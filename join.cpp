#include <algorithm>
#include <regex>
#include <sstream>

#include "Server.hpp"

// Generic error‐handling function: retrieves the client's nick and sends the
// error.
void Server::sendClientError(int clientFd, const std::string& errorCode,
                             const std::string& details) {
    // Retrieve the client's nick explicitly.
    std::string nick = _clients.at(getClientIndex(clientFd)).getNick();
    std::string msg =
        ":ft_irc " + errorCode + " " + nick + " " + details + "\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);
}

// Returns the index of the channel with the given name, or -1 if not found.
int Server::getChannelIndex(std::string name) {
    int index = 0;
    std::vector<Channel>::iterator it;
    for (it = _channels.begin(); it != _channels.end(); ++it) {
        if (it->getChannelName() == name)
            return index;
        index++;
    }
    return -1;
}

// Checks if the client (clientFd) is in the invited list.
bool Server::isClientInvited(int clientFd, std::vector<int>& invitedClients) {
    std::vector<int>::iterator it;
    for (it = invitedClients.begin(); it != invitedClients.end(); ++it) {
        if (*it == clientFd)
            return true;
    }
    return false;
}

// Checks if the client (clientFd) has already joined the channel.
bool Server::hasClientJoined(int clientFd, std::vector<int>& jointClients) {
    std::vector<int>::iterator it;
    for (it = jointClients.begin(); it != jointClients.end(); ++it) {
        if (*it == clientFd)
            return true;
    }
    return false;
}

// Checks if the provided key matches the channel's key.
bool Server::isKeyOk(int clientFd, Channel& channel, std::string key) {
    if (key == channel.getKey()) {
        return true;
    }
    sendClientError(clientFd, "475",
                    channel.getChannelName() + " :Cannot join channel (+k)");
    return false;
}

// If the channel is invite-only, ensures that the client is invited.
bool Server::isInviteOk(int clientFd, Channel& channel) {
    if (channel.isInviteOnly()) {
        if (!isClientInvited(clientFd, channel.getInvitedClients())) {
            sendClientError(
                clientFd, "473",
                channel.getChannelName() + " :Cannot join channel (+i)");
            return false;
        }
    }
    return true;
}

// Checks if the channel's client limit is not exceeded.
bool Server::isLimitOk(int clientFd, Channel& channel) {
    if (channel.getClientLimit() != -1) {
        if (static_cast<std::size_t>(channel.getClientLimit()) <=
            (channel.getJointClients().size() + channel.getOps().size())) {
            sendClientError(
                clientFd, "471",
                channel.getChannelName() + " :Cannot join channel (+l)");
            return false;
        }
    }
    return true;
}

// Validates that the channel name is well-formed (must start with '#' and
// contain no spaces or commas).
bool Server::isValidName(std::string channelName) {
    return std::regex_match(channelName, validNameRegex);
}

// Validates that the channel key is alphanumeric.
bool Server::isValidKey(std::string channelKey) {
    return std::regex_match(channelKey, validKeyRegex);
}

// Sends a welcome message to the client who just joined the channel.
// Also sends the channel topic (if any) and a list of current members.
void Server::welcome(int clientFd, Channel& channel, Client& client) {
    // Construct and send the JOIN notification message.
    // Format: ":<nick>!~<user>@<ip> JOIN <channel>\r\n"
    // This tells the client (and others) that the client has joined the
    // channel.
    std::string msg = ":" + client.getNick() + "!~" + client.getUser() + "@" +
                      client.getIPa() + " JOIN " + channel.getChannelName() +
                      "\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);

    // If the channel has a topic, send it to the client.
    // Format: ":ft_irc 332 <nick> <channel> :<topic>\r\n"
    // Numeric 332 is used in IRC to indicate the channel topic.
    if (!channel.getTopic().empty()) {
        msg = ":ft_irc 332 " + client.getNick() + " " +
              channel.getChannelName() + " :" + channel.getTopic() + "\r\n";
        send(clientFd, msg.c_str(), msg.length(), 0);
    }

    // Build and send the names list.
    // Format: ":ft_irc 353 <nick> @ <channel> :<name1> <name2> ...\r\n"
    // This lists all the users in the channel; operators are prefixed with '@'.
    msg = ":ft_irc 353 " + client.getNick() + " @ " + channel.getChannelName() +
          " :";

    // Append the nicknames of clients who have joined the channel.
    std::vector<int> jointClients = channel.getJointClients();
    for (std::vector<int>::iterator it = jointClients.begin();
         it != jointClients.end(); ++it) {
        int cIndex = getClientIndex(*it);
        msg += _clients.at(cIndex).getNick() + " ";
    }

    // Append operator names, prefixing each with '@'.
    std::vector<int> ops = channel.getOps();
    for (std::vector<int>::iterator it = ops.begin(); it != ops.end(); ++it) {
        int cIndex = getClientIndex(*it);
        msg += "@" + _clients.at(cIndex).getNick() + " ";
    }

    // Remove trailing space (if any) and add a newline.
    if (!jointClients.empty() || !ops.empty()) {
        msg.pop_back();  // Remove trailing space.
    }
    msg += "\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);

    // Send the end-of-NAMES list message.
    // Format: ":ft_irc 366 <nick> <channel> :End of /NAMES list\r\n"
    // Numeric 366 indicates the end of the NAMES reply.
    msg = ":ft_irc 366 " + client.getNick() + " " + channel.getChannelName() +
          " :End of /NAMES list\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);
}

// If a channel does not exist, this function creates a new channel.
void Server::newChannel(int clientFd, std::string channelName,
                        std::string channelKey) {
    std::string key = "";
    Client& client = _clients.at(getClientIndex(clientFd));

    if (!channelKey.empty()) {
        if (isValidKey(channelKey))
            key = channelKey;
        else {
            std::string msg = ":ft_irc 525 " + client.getNick() + " " +
                              channelName + " :Key is not well-formed\r\n";
            send(clientFd, msg.c_str(), msg.length(), 0);
            return;
        }
    }
    // Create a new channel; assume addChannel() adds it to _channels.
    Channel newChannel(channelName, clientFd, key);
    // addChannel(newChannel);
    // client.addOpChannel(
    //     channelName);  // Mark client as an operator of the new channel.
    welcome(clientFd, newChannel, client);
}

void Server::handleNonExistentChannel(int clientFd,
                                      const std::string& channelName,
                                      const std::string& channelKey) {
    if (!isValidName(channelName)) {
        sendClientError(clientFd, "476", channelName + " :Bad Channel Mask");
    } else {
        newChannel(clientFd, channelName, channelKey);
    }
}

// Validates join conditions: key, invite, and limit.
bool Server::validateJoinConditions(int clientFd, Channel& channel,
                                    const std::string& channelKey) {
    return isKeyOk(clientFd, channel, channelKey) &&
           isInviteOk(clientFd, channel) && isLimitOk(clientFd, channel);
}

// Performs the actual join: adds the client, removes invitation if present,
// sends the welcome message, and broadcasts the join message.
void Server::performJoin(int clientFd, Channel& channel) {
    Client& client = _clients.at(getClientIndex(clientFd));
    channel.addClient(clientFd);
    if (isClientInvited(clientFd, channel.getInvitedClients()))
        channel.removeInvite(clientFd);
    welcome(clientFd, channel, client);

    std::string msg = ":" + client.getNick() + "!" + client.getUser() + "@" +
                      client.getIPa() + " JOIN " + channel.getChannelName() +
                      "\r\n";
    channel.broadcast(msg, clientFd, false);
}

// Handles joining an existing channel.
void Server::joinChannel(int clientFd, std::string channelName,
                         std::string channelKey) {
    int channelIndex = getChannelIndex(channelName);  // returns -1 if not found
    if (channelIndex == -1) {
        handleNonExistentChannel(clientFd, channelName, channelKey);
        return;
    }

    auto& channel = _channels.at(channelIndex);
    if (hasClientJoined(clientFd, channel.getJointClients()))
        return;
    if (!validateJoinConditions(clientFd, channel, channelKey))
        return;
    performJoin(clientFd, channel);
}

// Processes the JOIN command for one or multiple channels.
void Server::join(int clientFd, std::string arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');

    // Command must be "JOIN".
    if (params.empty() || params.at(0) != "JOIN") {
        sendClientError(
            clientFd, "421",
            (params.empty() ? "" : params.at(0)) + " :Unknown command");
        return;
    }

    // There must be at least one channel specified.
    if (params.size() <= 1) {
        sendClientError(clientFd, "461", "JOIN :Not enough parameters");
        return;
    }

    // Parse channel names (and keys if provided).
    std::vector<std::string> channels;
    std::vector<std::string> keys;
    parser(params.at(1), channels, ',');
    if (params.size() > 2)
        parser(params.at(2), keys, ',');

    // Process each channel.
    for (std::size_t i = 0; i < channels.size(); ++i) {
        std::string key = (i < keys.size()) ? keys.at(i) : "";
        joinChannel(clientFd, channels.at(i), key);
    }
}
