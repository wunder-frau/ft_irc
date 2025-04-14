#include "Channel.hpp"
#include "Client.hpp"

#include <algorithm>  // for std::find and std::remove
#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

Channel::Channel() : _name(""), _topic(""), _inviteOnly(false), _operator(nullptr) {}

Channel::Channel(const std::string& name)
    : _name(name), _topic(""), _inviteOnly(false), _operator(nullptr)
{
}

Channel::~Channel()
{
    // No need to deallocate with vector
}

Channel::Channel(const Channel& other)
    : _name(other._name),
      _topic(other._topic),
      _inviteOnly(other._inviteOnly),
      _clients(other._clients),
      _operator(other._operator),
      _invited(other._invited)
{
}

Channel& Channel::operator=(const Channel& other)
{
    if (this != &other)
    {
        _name = other._name;
        _topic = other._topic;
        _inviteOnly = other._inviteOnly;
        _clients = other._clients;
        _operator = other._operator;
        _invited = other._invited;
    }
    return *this;
}

const std::string& Channel::getName() const { return _name; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }

const std::string& Channel::getTopic() const { return _topic; }

void Channel::setInviteOnly(bool inviteOnly) { _inviteOnly = inviteOnly; }

bool Channel::isInviteOnly() const { return _inviteOnly; }

bool Channel::isInvited(const std::string& nickname) const
{
    return _invited.find(nickname) != _invited.end();
}

void Channel::addInvited(const std::string& nickname) { _invited[nickname] = true; }

// Add a client to the channel.
// The first client becomes the channel operator.
void Channel::addClient(Client* client)
{
    if (client == nullptr)
        return;

    // Check if client is already in the channel
    if (isInChannel(client))
        return;

    _clients.push_back(client);

    // If no operator is assigned, make this client the operator.
    if (_operator == nullptr)
        _operator = client;
}

// Remove a client from the channel.
// If the client is the operator, assign a new operator if possible.
void Channel::removeClient(Client* client)
{
    if (client == nullptr)
        return;

    // Find and remove the client
    auto it = std::find(_clients.begin(), _clients.end(), client);
    if (it != _clients.end())
    {
        _clients.erase(it);

        // If the removed client was the operator, reassign the operator.
        if (_operator == client)
        {
            _operator = _clients.empty() ? nullptr : _clients[0];
        }
    }
}

bool Channel::isOperator(Client* client) const { return client == _operator; }

bool Channel::isInChannel(Client* client) const
{
    return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

// KICK command:
// Only the operator (sender) can kick a target from the channel.
bool Channel::kick(Client* sender, Client* target)
{
    if (sender == nullptr || target == nullptr)
        return false;

    // Verify that the sender is the operator.
    if (!isOperator(sender))
        return false;

    // Ensure that the target is actually in the channel.
    if (!isInChannel(target))
        return false;

    removeClient(target);
    std::cout << "Kicked " << target->getNick() << " from " << _name << std::endl;
    return true;
}

void Channel::broadcast(const std::string& message, Client* except)
{
    for (Client* client : _clients)
    {
        if (client != except)
        {
            send(client->getFd(), message.c_str(), message.length(), 0);
        }
    }
}

std::vector<Client*> Channel::getClients() const { return _clients; }
