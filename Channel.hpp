#pragma once
#include <string>
#include <vector>

class Channel {
private:
    std::string _name;
    std::string _key;
    bool _inviteOnly;
    int _clientLimit;
    std::string _topic;
    std::vector<int> _jointClients;
    std::vector<int> _ops;
    std::vector<int> _invitedClients;

public:
    Channel();
    Channel(const std::string& name, int creatorFd,
            const std::string& key = "");
    ~Channel();
    Channel(const Channel& other);
    Channel& operator=(const Channel& other);

    std::string getChannelName() const;
    std::string getKey() const;
    bool isInviteOnly() const;
    int getClientLimit() const;
    std::string getTopic() const;

    // Return references so they can be modified by the server code.
    std::vector<int>& getJointClients();
    std::vector<int>& getOps();
    std::vector<int>& getInvitedClients();

    // Methods to modify channel state:
    void addClient(int clientFd);
    void addOp(int clientFd);
    void removeInvite(int clientFd);

    // Broadcast a message to all clients in the channel.
    void broadcast(const std::string& msg, int senderFd, bool includeSender);
};
