#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Client.hpp"

class Channel {
public:
    // Default constructor
    Channel();

    // Parameterized constructor: creates a channel with a given name.
    Channel(const std::string& name);

    // Copy constructor (orthodox canonical form)
    Channel(const Channel& other);

    // Copy assignment operator (orthodox canonical form)
    Channel& operator=(const Channel& other);

    // Destructor (orthodox canonical form)
    ~Channel();

    // Getter for the channel name.
    const std::string& getName() const;

    // Topic management
    void setTopic(const std::string& topic);
    const std::string& getTopic() const;

    // Invite management
    void setInviteOnly(bool inviteOnly);
    bool isInviteOnly() const;
    bool isInvited(const std::string& nickname) const;
    void addInvited(const std::string& nickname);

    // Add a client to the channel.
    // If this is the first client, they become the operator.
    void addClient(Client* client);

    // Remove a client from the channel.
    // If the client was the operator, reassign operator to another client if
    // available.
    void removeClient(Client* client);

    // Check if the given client is the channel operator.
    bool isOperator(Client* client) const;

    // Check if client is in channel
    bool isInChannel(Client* client) const;

    // Kick a target client from the channel.
    // Only works if 'sender' is the operator.
    // Returns true if the kick was successful.
    bool kick(Client* sender, Client* target);

    // Broadcast a message to all clients in the channel
    void broadcast(const std::string& message, Client* except = nullptr);

    // Get list of clients in the channel
    std::vector<Client*> getClients() const;

    // MODE
    std::vector<int>& getOps() { return _ops; }

    void addOp(int op) { _ops.push_back(op); }
    void removeOp(int clientFd);
    void setTopicRestricted(bool restricted);
    bool isTopicRestricted() const;

    // KEY
    void setKey(const std::string& k) { _key = k; }
    const std::string& getModeKey() const;

    const std::string& getNormalizedName() const;
    bool isKeyed() const { return !_key.empty(); }

    // Client‐limit (+l) management. −1 means “no limit”.
    void setClientLimit(int limit) { _clientLimit = limit; }
    int getClientLimit() const { return _clientLimit; }

    void logClients() const;

private:
    std::string _name;
    std::string normalizedName;
    std::string _topic;
    bool _inviteOnly;
    std::vector<int> _ops;  // moderator clients

    // Using vector for better memory management
    std::vector<Client*> _clients;
    Client* _operator;  // Pointer to the current channel operator.
    std::string displayName;
    // Keep track of invited users (for invite-only channels)
    std::map<std::string, bool> _invited;

    // int _clientLimit;  // –1 means unlimited
    bool _topicRestricted = false;
    std::string _key = "";
    int _clientLimit = -1;  // –1 means unlimited
};

#endif  // CHANNEL_HPP
