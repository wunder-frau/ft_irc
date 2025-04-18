#include <unordered_set>

#include "Server.hpp"
#include "utils.hpp"

bool Server::hasOpRights(int clientFd, std::string channelName) {
    int chIndex = getChannelIndex(channelName);
    std::vector<int>& ops = _channels.at(chIndex).getOps();
    std::vector<int>::iterator result =
        std::find(ops.begin(), ops.end(), clientFd);

    if (result != ops.end())
        return (true);
    return (false);
}

bool Server::isClient(std::string nick) {
    for (std::vector<Client>::iterator it = _clients.begin();
         it != _clients.end(); std::advance(it, 1)) {
        if ((*it).getNick() == nick)
            return (true);
    }
    return (false);
}

int Server::getChannelIndex(std::string name) {
    int index = 0;
    for (std::vector<Channel>::iterator it = _channels.begin();
         it != _channels.end(); std::advance(it, 1)) {
        if (it->getName() == name) {
            return (index);
        }
        index++;
    }
    return (-1);
}

bool Server::verifyParams(int clientFd, std::vector<std::string>& params) {
    // Must have at least a target
    if (params.size() < 2)
        return false;

    std::string& target = params[1];

    // Target must be an existing channel or a known nick
    if (getChannelIndex(target) < 0 && !isClient(target)) {
        sendError(clientFd, "403", target, "No such channel or nick");
        return false;
    }

    // If there is a mode flag, it must be one of the allowed set
    if (params.size() > 2) {
        static const std::unordered_set<std::string> validFlags = {
            "+i", "-i", "+t", "-t", "+k", "-k", "+o", "-o", "+l", "-l"};
        if (!validFlags.contains(params[2]))
            return false;
    }

    return true;
}

void Server::returnChannelMode(int clientFd, Channel& channel) {
    // Build the mode‐flags substring
    std::string modes = "+";
    if (channel.isInviteOnly())
        modes += 'i';
    if (channel.isTopicRestricted())  // ← add this check
        modes += 't';
    // Compose and send the IRC reply
    std::string msg = ":ft_irc 324 " +
                      _clients.at(getClientIndex(clientFd)).getNick() + " " +
                      channel.getName() + " " + modes + "\r\n";
    send(clientFd, msg.c_str(), msg.size(), 0);
}

bool Server::applyChannelMode(Client* client, Channel& channel,
                              const std::string& flag,
                              const std::vector<std::string>& params) {
    (void)params;  // Unused parameter
    bool adding = (flag[0] == '+');
    char modeChar = flag[1];
    std::string modeStr = flag;

    switch (modeChar) {
        case 'i': {
            channel.setInviteOnly(adding);
            std::string msg = ":" + client->getNick() + "!~" +
                              client->getUser() + "@" + client->getIPa() +
                              " MODE " + channel.getName() + " " + modeStr +
                              "\r\n";
            channel.broadcast(msg);
            return true;
        }
        case 't': {
            // 1) what is the channel’s current +t status?
            bool currentlyRestricted = channel.isTopicRestricted();

            // 2) if they’re asking to +t but it’s already +t, or -t but it’s
            // already -t, skip
            if (adding == currentlyRestricted) {
                return true;  // we “handled” it, but no broadcast needed
            }

            // 3) actually flip the flag and broadcast the MODE change
            channel.setTopicRestricted(adding);
            std::string msg = ":" + client->getNick() + "!~" +
                              client->getUser() + "@" + client->getIPa() +
                              " MODE " + channel.getName() + " " + modeStr +
                              "\r\n";
            channel.broadcast(msg);
            return true;
        }
        // case 'k': {
        //     // requires a key param in params[3]
        //     if (params.size() < 4)
        //         return false;
        //     if (adding)
        //         channel.setKey(params[3]);
        //     else
        //         channel.setKey("");
        //     std::string msg = ":" + client->getNick() + "!~" +
        //                       client->getUser() + "@" + client->getIPa() +
        //                       " MODE " + channel.getName() + " " + modeStr +
        //                       (adding ? " " + params[3] : "") + "\r\n";
        //     channel.broadcast(msg);
        //     return true;
        // }
        // case 'l': {
        //     // requires a limit in params[3]
        //     if (params.size() < 4)
        //         return false;
        //     int limit = adding ? std::stoi(params[3]) : -1;
        //     channel.setClientLimit(limit);
        //     std::string msg = ":" + client->getNick() + "!~" +
        //                       client->getUser() + "@" + client->getIPa() +
        //                       " MODE " + channel.getName() + " " + modeStr +
        //                       (adding ? " " + params[3] : "") + "\r\n";
        //     channel.broadcast(msg);
        //     return true;
        // }
        // case 'o': {
        //     // requires a nick in params[3]
        //     if (params.size() < 4)
        //         return false;
        //     Client* target = getClientObjByNick(params[3]);
        //     if (!target)
        //         return false;
        //     if (adding)
        //         channel.addOp(target->getFd());
        //     else
        //         channel.removeOp(target->getFd());
        //     std::string msg = ":" + client->getNick() + "!~" +
        //                       client->getUser() + "@" + client->getIPa() +
        //                       " MODE " + channel.getName() + " " + modeStr +
        //                       " " + params[3] + "\r\n";
        //     channel.broadcast(msg);
        //     return true;
        // }
        default:
            return false;  // unknown flag
    }
}

void Server::setMode(int clientFd, std::vector<std::string>& params) {
    Client* client = getClientObjByFd(clientFd);
    if (!client)
        return;

    // Query only: MODE #chan
    if (params.size() == 2) {
        Channel* chan = findChannel(params[1]);
        if (chan)
            returnChannelMode(clientFd, *chan);
        return;
    }

    // Validate & op rights
    if (!verifyParams(clientFd, params) || !hasOpRights(clientFd, params[1])) {
        sendError(clientFd, "482", client->getNick(),
                  params[1] + " :You're not channel operator");
        return;
    }

    Channel& channel = _channels.at(getChannelIndex(params[1]));
    const std::string& flag = params[2];

    if (!applyChannelMode(client, channel, flag, params)) {
        // either unknown flag or missing args
        char bad = flag.size() > 1 ? flag[1] : '?';
        sendError(clientFd, "472", client->getNick(),
                  std::string(1, bad) + " :is unknown mode char to me");
    }
}

void Server::handleMode(int clientFd, const std::string& arg) {
    Client* client = getClientObjByFd(clientFd);
    if (!client) {
        return;
    }

    // Parse MODE command
    std::vector<std::string> params;
    parser(arg, params, ' ');
    for (std::string& p : params) p = trimWhitespace(p);

    if (params.size() < 2) {
        sendError(clientFd, "461", client->getNick(),
                  "MODE :Not enough parameters");
        return;
    }
    std::cout << "[in progress] LOOK here NOW setMode: target='" << params[1]
              << "'" << std::endl;
    setMode(clientFd, params);
}
