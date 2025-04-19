#include "ModeHandler.hpp"

#include "ModeUtils.hpp"
#include "utils.hpp"

bool handleInviteOnlyMode(Client* client, Channel& channel, bool adding) {
    channel.setInviteOnly(adding);
    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                      client->getIPa() + " MODE " + channel.getName() + " " +
                      (adding ? "+i" : "-i") + "\r\n";
    channel.broadcast(msg);
    return true;
}

bool handleTopicRestrictMode(Client* client, Channel& channel, bool adding) {
    if (adding == channel.isTopicRestricted())
        return true;
    channel.setTopicRestricted(adding);
    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                      client->getIPa() + " MODE " + channel.getName() + " " +
                      (adding ? "+t" : "-t") + "\r\n";
    channel.broadcast(msg);
    return true;
}

bool handleKeyMode(Client* client, Channel& channel, bool adding,
                   const std::vector<std::string>& params) {
    if (adding) {
        if (params.size() < 4) {
            sendError(client->getFd(), "461", client->getNick(),
                      "MODE :Not enough parameters");
            return true;
        }
        const std::string& key = params[3];
        if (!isValidKey(key)) {
            sendError(client->getFd(), "525", client->getNick(),
                      channel.getName() + " :Key is not well-formed");
            return true;
        }
        channel.setKey(key);
    } else {
        channel.setKey("");
    }
    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                      client->getIPa() + " MODE " + channel.getName() + " " +
                      (adding ? "+k " + params[3] : "-k") + "\r\n";
    channel.broadcast(msg);
    return true;
}

bool handleLimitMode(Client* client, Channel& channel, bool adding,
                     const std::vector<std::string>& params) {
    if (adding) {
        if (params.size() < 4) {
            sendError(client->getFd(), "461", client->getNick(),
                      "MODE :Not enough parameters");
            return true;
        }
        int limit = std::stoi(params[3]);
        channel.setClientLimit(limit);
    } else {
        channel.setClientLimit(-1);
    }
    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                      client->getIPa() + " MODE " + channel.getName() + " " +
                      (adding ? "+l " + params[3] : "-l") + "\r\n";
    channel.broadcast(msg);
    return true;
}

bool handleOpMode(Server& server, Client* client, Channel& channel, bool adding,
                  const std::vector<std::string>& params) {
    if (params.size() < 4) {
        sendError(client->getFd(), "461", client->getNick(),
                  "MODE :Not enough parameters");
        return true;
    }

    std::string targetNick = params[3];
    Client* target = server.getClientObjByNick(targetNick);
    if (!target) {
        sendError(client->getFd(), "401", client->getNick(),
                  targetNick + " :No such nick/channel");
        return true;
    }

    if (!channel.isInChannel(target)) {
        sendError(client->getFd(), "441", client->getNick(),
                  targetNick + " " + channel.getName() +
                      " :They aren't on that channel");
        return true;
    }

    if (adding)
        channel.addOp(target->getFd());
    else
        channel.removeOp(target->getFd());

    std::string msg = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                      client->getIPa() + " MODE " + channel.getName() + " " +
                      (adding ? "+o " : "-o ") + targetNick + "\r\n";
    channel.broadcast(msg);
    return true;
}
