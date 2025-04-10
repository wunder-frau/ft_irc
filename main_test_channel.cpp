#include <iostream>
#include <vector>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

// Dummy send() implementation for testing purposes
ssize_t send(int socket, const void* buffer, size_t length, int /*flags*/) {
    std::cout << "[Socket " << socket << "] "
              << std::string(static_cast<const char*>(buffer), length);
    return length;
}

int main() {
    // Create a server instance
    Server server(6667, "secret");
    
    // Create test clients
    Client* creator = new Client(100, "192.168.1.100");
    creator->setNickname("Creator");
    creator->setUsername("creator_user");
    creator->setPassword("secret");
    creator->setAsRegistered();
    
    Client* client1 = new Client(200, "192.168.1.200");
    client1->setNickname("Client1");
    client1->setUsername("client1_user");
    client1->setPassword("secret");
    client1->setAsRegistered();
    
    Client* client2 = new Client(300, "192.168.1.300");
    client2->setNickname("Client2");
    client2->setUsername("client2_user");
    client2->setPassword("secret");
    client2->setAsRegistered();
    
    // Add clients to server
    server.addClient(*creator);
    server.addClient(*client1);
    server.addClient(*client2);
    
    // Create a channel named "#test"
    Channel* channel = new Channel("#test");
    
    // Add the creator as the first client (becomes operator automatically)
    channel->addClient(creator);
    
    std::cout << "Channel created: " << channel->getName() << std::endl;
    std::cout << "Is invite only: " << (channel->isInviteOnly() ? "Yes" : "No")
              << std::endl;
    std::cout << "Topic: " << (channel->getTopic().empty() ? "<no topic>" : channel->getTopic()) << std::endl << std::endl;
    
    // Check that the creator is added as both an operator and a joined client
    std::vector<Client*> members = channel->getClients();
    
    std::cout << "Members: ";
    for (size_t i = 0; i < members.size(); ++i) {
        std::cout << members[i]->getNick() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Operators: ";
    for (size_t i = 0; i < members.size(); ++i) {
        if (channel->isOperator(members[i])) {
            std::cout << members[i]->getNick() << " ";
        }
    }
    std::cout << std::endl << std::endl;
    
    // Add a new client to the channel
    channel->addClient(client1);
    std::cout << "Added client " << client1->getNick() << "." << std::endl;
    
    // Add another client to the channel
    channel->addClient(client2);
    std::cout << "Added client " << client2->getNick() << "." << std::endl;
    
    std::cout << "Members after update: ";
    members = channel->getClients();
    for (size_t i = 0; i < members.size(); ++i) {
        std::cout << members[i]->getNick() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Operators after update: ";
    for (size_t i = 0; i < members.size(); ++i) {
        if (channel->isOperator(members[i])) {
            std::cout << members[i]->getNick() << " ";
        }
    }
    std::cout << std::endl << std::endl;
    
    // Test invite functionality
    Client* invitedClient = new Client(400, "192.168.1.400");
    invitedClient->setNickname("InvitedClient");
    invitedClient->setUsername("invited_user");
    invitedClient->setPassword("secret");
    invitedClient->setAsRegistered();
    
    server.addClient(*invitedClient);
    
    std::cout << "Inviting " << invitedClient->getNick() << " to the channel" << std::endl;
    channel->addInvited(invitedClient->getNick());
    
    std::cout << "Is " << invitedClient->getNick() << " invited? " 
              << (channel->isInvited(invitedClient->getNick()) ? "Yes" : "No") << std::endl;
    
    // Test broadcast
    std::cout << "Broadcasting message to channel:" << std::endl;
    channel->broadcast("Hello, channel!", creator);
    
    // Test topic setting
    std::cout << "Setting channel topic:" << std::endl;
    channel->setTopic("This is a test channel");
    std::cout << "New topic: " << channel->getTopic() << std::endl << std::endl;
    
    // Test mode setting
    std::cout << "Setting channel to invite-only:" << std::endl;
    channel->setInviteOnly(true);
    std::cout << "Is invite only: " << (channel->isInviteOnly() ? "Yes" : "No") << std::endl << std::endl;
    
    // Test removing a client
    std::cout << "Removing " << client1->getNick() << " from the channel:" << std::endl;
    channel->removeClient(client1);
    
    std::cout << "Members after removal: ";
    members = channel->getClients();
    for (size_t i = 0; i < members.size(); ++i) {
        std::cout << members[i]->getNick() << " ";
    }
    std::cout << std::endl << std::endl;
    
    // Clean up
    delete channel;
    delete creator;
    delete client1;
    delete client2;
    delete invitedClient;
    
    return 0;
}
