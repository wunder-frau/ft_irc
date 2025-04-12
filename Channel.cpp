#include "Channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _topic("") {}

const std::string& Channel::getName() const { return _name; }

const std::string& Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }

void Channel::addClient(Client* client) { _clients.insert(client); }

void Channel::removeClient(Client* client)
{
    _clients.erase(client);
    _operators.erase(client);
}

bool Channel::hasClient(Client* client) const { return _clients.count(client) > 0; }

void Channel::addOperator(Client* client) { _operators.insert(client); }

bool Channel::isOperator(Client* client) const { return _operators.count(client) > 0; }

const std::set<Client*>& Channel::getClients() const { return _clients; }
