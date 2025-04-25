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
    if (!client)
        return;

    std::vector<std::string> params;
    parser(arg, params, ' ');
    for (std::string& p : params) p = trimWhitespace(p);

    if (params.size() < 2) {
        sendError(clientFd, "461", client->getNick(),
                  "MODE :Not enough parameters");
        return;
    }

    std::string target = params[1];

    std::cout << "[handleMode] Target='" << target << "'" << std::endl;

    // If target starts with #, &, +, ! → it's a channel
    if (!target.empty() && (target[0] == '#' || target[0] == '&' ||
                            target[0] == '+' || target[0] == '!')) {
        setMode(clientFd, params);  // Handle channel modes (+o, +k, etc.)
    } else {
        // Target is a nickname (user mode like +i)
        Client* targetClient = getClientObjByNick(target);
        if (!targetClient) {
            sendError(clientFd, "401", client->getNick(),
                      target + " :No such nick");
            return;
        }

        if (params.size() >= 3) {
            std::string modes = params[2];
            bool adding = true;  // true = + mode, false = - mode

            for (char mode : modes) {
                if (mode == '+') {
                    adding = true;
                    continue;
                }
                if (mode == '-') {
                    adding = false;
                    continue;
                }

                if (mode == 'i') {
                    if (adding) {
                        targetClient->setInvisible(true);
                        std::cout
                            << "[handleMode] Setting +i (invisible) for user '"
                            << targetClient->getNick() << "'" << std::endl;
                    } else {
                        targetClient->setInvisible(false);
                        std::cout
                            << "[handleMode] Removing +i (invisible) for user '"
                            << targetClient->getNick() << "'" << std::endl;
                    }
                } else {
                    sendError(clientFd, "501", client->getNick(),
                              ":Unknown mode flag");
                    return;  // Stop on unknown mode
                }
            }
        } else {
            sendError(clientFd, "461", client->getNick(),
                      "MODE :Not enough parameters");
            return;
        }

        // Respond to the client: confirm mode change
        std::string reply = ":" + client->getNick() + " MODE " + target + " " +
                            params[2] + "\r\n";
        send(clientFd, reply.c_str(), reply.length(), 0);
    }
}
