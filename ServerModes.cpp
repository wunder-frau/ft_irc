#include <unordered_set>

#include "Server.hpp"
#include "modes/ModeHandler.hpp"
#include "modes/ModeUtils.hpp"
#include "utils.hpp"

bool Server::applyChannelMode(Client* client, Channel& channel,
                              const std::string& flag,
                              const std::vector<std::string>& params) {
    bool adding = (flag[0] == '+');
    char modeChar = flag[1];

    switch (modeChar) {
        case 'i':
            return handleInviteOnlyMode(client, channel, adding);
        case 't':
            return handleTopicRestrictMode(client, channel, adding);
        case 'k':
            return handleKeyMode(client, channel, adding, params);
        case 'l':
            return handleLimitMode(client, channel, adding, params);
        case 'o':
            return handleOpMode(*this, client, channel, adding, params);
        default:
            return false;
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
            returnChannelMode(*this, clientFd, *chan);
        return;
    }

    // Validate & check operator rights
    if (!verifyParams(*this, clientFd, params) ||
        !hasOpRights(*this, clientFd, params[1])) {
        sendError(clientFd, "482", client->getNick(),
                  params[1] + " :You're not channel operator");
        return;
    }

    Channel& channel = _channels.at(getChannelIndex(*this, params[1]));
    const std::string& flag = params[2];

    if (!applyChannelMode(client, channel, flag, params)) {
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