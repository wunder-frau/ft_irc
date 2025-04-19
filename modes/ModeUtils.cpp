#include "ModeUtils.hpp"

#include "Server.hpp"
#include "utils.hpp"

bool isValidKey(const std::string& s) {
    return !s.empty() && s.find(' ') == std::string::npos;
}

bool setKey(Server& server, int clientFd, Channel& channel,
            const std::vector<std::string>& params) {
    bool adding = (params[2][0] == '+');
    if (adding) {
        if (params.size() < 4) {
            sendError(clientFd, "461",
                      server.getClientObjByFd(clientFd)->getNick(),
                      "MODE :Not enough parameters");
            return false;
        }
        const std::string& key = params[3];
        if (!isValidKey(key)) {
            sendError(clientFd, "525",
                      server.getClientObjByFd(clientFd)->getNick(),
                      channel.getName() + " :Key is not well‑formed");
            return false;
        }
        channel.setKey(key);
    } else {
        channel.setKey("");
    }

    Client* client = server.getClientObjByFd(clientFd);
    std::string modeStr = params[2] + (adding ? " " + params[3] : "");
    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                      client->getIPa() + " MODE " + channel.getName() + " " +
                      modeStr + "\r\n";
    channel.broadcast(msg);
    return true;
}

bool hasOpRights(Server& server, int clientFd, const std::string& channelName) {
    int chIndex = getChannelIndex(server, channelName);
    std::vector<int>& ops = server.getChannels().at(chIndex).getOps();
    return std::find(ops.begin(), ops.end(), clientFd) != ops.end();
}

bool isClient(Server& server, const std::string& nick) {
    for (std::vector<Client>::const_iterator it = server.getClients().begin();
         it != server.getClients().end(); ++it) {
        if (it->getNick() == nick)
            return true;
    }
    return false;
}

int getChannelIndex(Server& server, const std::string& name) {
    const std::vector<Channel>& chans = server.getChannels();
    for (size_t i = 0; i < chans.size(); ++i) {
        if (chans[i].getName() == name)
            return static_cast<int>(i);
    }
    return -1;
}

bool verifyParams(Server& server, int clientFd,
                  std::vector<std::string>& params) {
    if (params.size() < 2)
        return false;

    std::string& target = params[1];

    if (getChannelIndex(server, target) < 0 && !isClient(server, target)) {
        sendError(clientFd, "403", target, "No such channel or nick");
        return false;
    }

    if (params.size() > 2) {
        static const std::unordered_set<std::string> validFlags = {
            "+i", "-i", "+t", "-t", "+k", "-k", "+o", "-o", "+l", "-l"};
        if (!validFlags.contains(params[2]))
            return false;
    }

    return true;
}

void returnChannelMode(Server& server, int clientFd, Channel& channel) {
    std::string modes = "+";
    if (channel.isInviteOnly())
        modes += 'i';
    if (channel.isTopicRestricted())
        modes += 't';
    if (channel.isKeyed())
        modes += 'k';
    if (channel.getClientLimit() > -1)
        modes += 'l';

    std::string msg = ":ft_irc 324 " +
                      server.getClientObjByFd(clientFd)->getNick() + " " +
                      channel.getName() + " " + modes + "\r\n";
    send(clientFd, msg.c_str(), msg.size(), 0);
}
