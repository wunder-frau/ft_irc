#include "Channel.hpp"

#include <algorithm>  // for std::find and std::remove
#include <iostream>

#include "Client.hpp"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

Channel::Channel()
    : _name(""),
      _topic(""),
      _inviteOnly(false),
      _ops(),
      _clients(),
      _operator(nullptr),
      _invited(),
      _topicRestricted(false),
      _key(""),
      _clientLimit(-1) {}

Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
      _inviteOnly(false),
      _ops(),
      _clients(),
      _operator(nullptr),
      _invited(),
      _topicRestricted(false),
      _key(""),
      _clientLimit(-1) {}

Channel::~Channel() {}

Channel::Channel(const Channel& other)
    : _name(other._name),
      _topic(other._topic),
      _inviteOnly(other._inviteOnly),
      _ops(other._ops),
      _clients(other._clients),
      _operator(other._operator),
      _invited(other._invited),
      _topicRestricted(other._topicRestricted),
      _key(other._key),
      _clientLimit(other._clientLimit) {}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        _name = other._name;
        _topic = other._topic;
        _inviteOnly = other._inviteOnly;
        _ops = other._ops;
        _clients = other._clients;
        _operator = other._operator;
        _invited = other._invited;
        _topicRestricted = other._topicRestricted;
        _key = other._key;
        _clientLimit = other._clientLimit;
    }
    return *this;
}

const std::string& Channel::getName() const { return _name; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }

const std::string& Channel::getTopic() const { return _topic; }

void Channel::setInviteOnly(bool inviteOnly) { _inviteOnly = inviteOnly; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

bool Channel::isInvited(const std::string& nickname) const {
    return _invited.find(nickname) != _invited.end();
}

void Channel::addInvited(const std::string& nickname) {
    _invited[nickname] = true;
}

// Add a client to the channel.
// The first client becomes the channel operator.
void Channel::addClient(Client* client) {
    if (!client)
        return;
    if (isInChannel(client))
        return;

    _clients.push_back(client);

    // 1) If no operator yet, make this new client the operator …
    if (_operator == nullptr) {
        _operator = client;

        // 2) … and record their fd in the ops list:
        _ops.push_back(client->getFd());
    }
}

// Remove a client from the channel.
// If the client is the operator, assign a new operator if possible.
void Channel::removeClient(Client* client) {
    if (client == nullptr)
        return;

    // Find and remove the client
    auto it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end()) {
        _clients.erase(it);

        // If the removed client was the operator, reassign the operator.
        if (_operator == client) {
            _operator = _clients.empty() ? nullptr : _clients[0];
        }
    }
}

bool Channel::isOperator(Client* client) const {
    int fd = client->getFd();
    return std::find(_ops.begin(), _ops.end(), fd) != _ops.end();
}

bool Channel::isInChannel(Client* client) const {
    return std::find(_clients.begin(), _clients.end(), client) !=
           _clients.end();
}

// KICK command:
// Only the operator (sender) can kick a target from the channel.
bool Channel::kick(Client* sender, Client* target) {
    if (sender == nullptr || target == nullptr)
        return false;

    // Verify that the sender is the operator.
    if (!isOperator(sender))
        return false;

    // Ensure that the target is actually in the channel.
    if (!isInChannel(target))
        return false;

    removeClient(target);
    std::cout << "Kicked " << target->getNick() << " from " << _name
              << std::endl;
    return true;
}

void Channel::broadcast(const std::string& message, Client* except) {
    // 0) Dump the overall call
    std::cout << "[DEBUG broadcast] channel='" << _name
              << "' except_fd=" << (except ? except->getFd() : -1)
              << " message=\"" << message << "\"" << std::endl;

    for (Client* member : _clients) {
        // 1) Null‐pointer check
        if (member == nullptr) {
            std::cout << "[DEBUG broadcast] skipping null member pointer"
                      << std::endl;
            continue;
        }

        int fd = member->getFd();
        std::cout << "[DEBUG broadcast] considering member '"
                  << member->getNick() << "' fd=" << fd << std::endl;

        // 2) Don’t echo to the ‘except’ (sender)
        if (member == except) {
            std::cout << "[DEBUG broadcast] skipping except member '"
                      << member->getNick() << "'" << std::endl;
            continue;
        }

        // 3) Filter out invalid file descriptors
        if (fd <= 0) {
            std::cout << "[WARNING broadcast] invalid fd for '"
                      << member->getNick() << "': " << fd << std::endl;
            continue;
        }

        // 4) Do the send and log how many bytes went out
        ssize_t sent = ::send(fd, message.c_str(), message.size(), 0);
        if (sent < 0) {
            std::perror("[ERROR broadcast] send() failed");
        } else {
            std::cout << "[DEBUG broadcast] sent " << sent << " bytes to '"
                      << member->getNick() << "' (fd=" << fd << ")"
                      << std::endl;
        }
    }
}

std::vector<Client*> Channel::getClients() const { return _clients; }

void Channel::setTopicRestricted(bool restricted) {
    _topicRestricted = restricted;
}

bool Channel::isTopicRestricted() const { return _topicRestricted; }

// Remove an fd from the op list
void Channel::removeOp(int clientFd) {
    _ops.erase(std::remove(_ops.begin(), _ops.end(), clientFd), _ops.end());
}