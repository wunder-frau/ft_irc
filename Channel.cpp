#include "Channel.hpp"

#include <algorithm>  // for std::find and std::remove
#include <iostream>

Channel::Channel()
    : _name(""), _key(""), _inviteOnly(false), _clientLimit(-1), _topic("") {}
Channel::Channel(const std::string& name, int creatorFd, const std::string& key)
    : _name(name), _key(key), _inviteOnly(false), _clientLimit(-1), _topic("") {
    // When a channel is created, the file descriptor representing the creator
    // is added to both lists, ensuring that the creator is both an operator and
    // a member of the channel from the start.
    _ops.push_back(creatorFd);
    _jointClients.push_back(creatorFd);
}

Channel::~Channel() {}

Channel::Channel(const Channel& other)
    : _name(other._name),
      _key(other._key),
      _inviteOnly(other._inviteOnly),
      _clientLimit(other._clientLimit),
      _topic(other._topic),
      _jointClients(other._jointClients),
      _ops(other._ops),
      _invitedClients(other._invitedClients) {}

Channel& Channel::operator=(const Channel& other) {
    if (this != &other) {
        _name = other._name;
        _key = other._key;
        _inviteOnly = other._inviteOnly;
        _clientLimit = other._clientLimit;
        _topic = other._topic;
        _jointClients = other._jointClients;
        _ops = other._ops;
        _invitedClients = other._invitedClients;
    }
    return *this;
}

std::string Channel::getChannelName() const { return _name; }

std::string Channel::getKey() const { return _key; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

int Channel::getClientLimit() const { return _clientLimit; }

std::string Channel::getTopic() const { return _topic; }

// Return references for modification
std::vector<int>& Channel::getJointClients() { return _jointClients; }

std::vector<int>& Channel::getOps() { return _ops; }

std::vector<int>& Channel::getInvitedClients() { return _invitedClients; }

// Methods to modify channel state
void Channel::addClient(int clientFd) {
    // Add client FD to the list of joined clients.
    _jointClients.push_back(clientFd);
}

void Channel::addOp(int clientFd) {
    // Add client FD to operators if not already present.
    if (std::find(_ops.begin(), _ops.end(), clientFd) == _ops.end()) {
        _ops.push_back(clientFd);
    }
}

void Channel::removeInvite(int clientFd) {
    // Remove the client FD from the invited clients list.
    _invitedClients.erase(
        std::remove(_invitedClients.begin(), _invitedClients.end(), clientFd),
        _invitedClients.end());
}

// Broadcast a message to all clients in the channel.
// For this minimal implementation, we simply print the message to the console.
void Channel::broadcast(const std::string& msg, int senderFd,
                        bool includeSender) {
    (void)includeSender;
    std::cout << "Broadcasting in channel \"" << _name << "\" from sender FD "
              << senderFd << ":\n"
              << msg << std::endl;
}
