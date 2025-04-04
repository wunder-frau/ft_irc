#include "Server.hpp"

size_t Server::getClientIndex(int clientFd) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i].getFd() == clientFd)
            return i;
    }
    throw std::runtime_error("Client with fd " + std::to_string(clientFd) +
                             " not found");
}

// In Server.cpp, implement the new function:
void Server::broadcastAndUpdateNickname(int clientFd, size_t clientIndex,
                                        const std::string &newNick) {
    const std::string currentNick = _clients.at(clientIndex).getNick();
    std::string msg =
        ":" + currentNick + "!~" + _clients.at(clientIndex).getUser() + "@" +
        _clients.at(clientIndex).getIPa() + " NICK " + newNick + "\r\n";

    // TODO: add channels here:
    // for (auto &channel : _channels) {
    //     }
    // }

    // Send confirmation to the client and update their nickname.
    send(clientFd, msg.c_str(), msg.length(), 0);
    _clients.at(clientIndex).setNickname(newNick);
}

void Server::sendError(int clientFd, const std::string &errorCode,
                       const std::string &nick, const std::string &details) {
    std::string msg =
        ":ft_irc " + errorCode + " " + nick + " " + details + "\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);
}

void Server::validateNick(int clientFd, const std::string &newNick) {
    size_t clientIndex = getClientIndex(clientFd);
    const std::string currentNick = _clients.at(clientIndex).getNick();

    // 1. Check if the nickname is empty.
    if (newNick.empty()) {
        sendError(clientFd, "432", currentNick,
                  newNick + " :Erroneus nickname (empty)");
        return;
    }
    // 2. Check if the nickname contains any spaces.
    if (newNick.find(' ') != std::string::npos) {
        sendError(clientFd, "432", currentNick,
                  newNick + " :Erroneus nickname (contains space)");
        return;
    }
    // 3. Check if the nickname matches the forbidden pattern.
    if (std::regex_match(newNick, incorrectRegex)) {
        sendError(clientFd, "432", currentNick,
                  newNick + " :Erroneus nickname (invalid pattern)");
        return;
    }
    // 4. Check if the nickname is unique.
    if (!isUniqueNick(newNick)) {
        sendError(clientFd, "433", currentNick, newNick);
        return;
    }

    broadcastAndUpdateNickname(clientFd, clientIndex, newNick);
}

void Server::nick(int clientFd, std::string arg) {
    std::vector<std::string> params;
    parser(arg, params, ' ');

    // Check that the command is "NICK".
    if (params.empty() || params.at(0) != "NICK") {
        sendError(clientFd, "421",
                  _clients.at(getClientIndex(clientFd)).getUser(),
                  (params.empty() ? " :Unknown command"
                                  : params.at(0) + " :Unknown command"));
        return;
    }

    // Ensure a new nickname is provided.
    if (params.size() <= 1) {
        sendError(clientFd, "431",
                  _clients.at(getClientIndex(clientFd)).getNick(),
                  ":No nickname given");
        return;
    }

    // If the new nickname is the same as the current one, do nothing.
    if (_clients.at(getClientIndex(clientFd)).getNick() == params.at(1))
        return;

    validateNick(clientFd, params.at(1));
}