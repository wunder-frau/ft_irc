#pragma once

#include <string>
#include <set>

class Client;

class Channel
{
   public:
    Channel(const std::string& name);

    const std::string& getName() const;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);

    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;

    void addOperator(Client* client);
    bool isOperator(Client* client) const;

    const std::set<Client*>& getClients() const;

   private:
    std::string _name;
    std::string _topic;

    std::set<Client*> _clients;
    std::set<Client*> _operators;
};
