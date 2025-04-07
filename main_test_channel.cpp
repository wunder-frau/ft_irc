#include <iostream>
#include <vector>

#include "Channel.hpp"

int main() {
    // Create a channel named "#test" with creator FD 100 and no key.
    Channel channel("#test", 100);

    std::cout << "Channel created: " << channel.getChannelName() << std::endl;
    std::cout << "Channel key: " << channel.getKey() << std::endl;
    std::cout << "Is invite only: " << (channel.isInviteOnly() ? "Yes" : "No")
              << std::endl;
    std::cout << "Client limit: " << channel.getClientLimit() << std::endl;
    std::cout << "Topic: " << channel.getTopic() << std::endl << std::endl;

    // Check that the creator (FD 100) is added as both an operator and a joined
    // client.
    std::vector<int>& ops = channel.getOps();
    std::vector<int>& jointClients = channel.getJointClients();

    std::cout << "Operators: ";
    for (size_t i = 0; i < ops.size(); ++i) {
        std::cout << ops[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Joined Clients: ";
    for (size_t i = 0; i < jointClients.size(); ++i) {
        std::cout << jointClients[i] << " ";
    }
    std::cout << std::endl << std::endl;

    // Add a new client (FD 200) to the channel.
    channel.addClient(200);
    std::cout << "Added client FD 200." << std::endl;

    // Add another operator (FD 300) to the channel.
    channel.addOp(300);
    std::cout << "Added operator FD 300." << std::endl;

    std::cout << "Operators after update: ";
    for (size_t i = 0; i < ops.size(); ++i) {
        std::cout << ops[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Joined Clients after update: ";
    for (size_t i = 0; i < jointClients.size(); ++i) {
        std::cout << jointClients[i] << " ";
    }
    std::cout << std::endl << std::endl;

    // Simulate invitation list: add FDs 400 and 500.
    std::vector<int>& invited = channel.getInvitedClients();
    invited.push_back(400);
    invited.push_back(500);

    std::cout << "Invited Clients: ";
    for (size_t i = 0; i < invited.size(); ++i) {
        std::cout << invited[i] << " ";
    }
    std::cout << std::endl;

    // Remove invite for client FD 400.
    channel.removeInvite(400);
    std::cout << "Invited Clients after removing FD 400: ";
    for (size_t i = 0; i < invited.size(); ++i) {
        std::cout << invited[i] << " ";
    }
    std::cout << std::endl << std::endl;

    // Test broadcast: simulate sending a message from FD 100.
    channel.broadcast("Hello, channel!", 100, true);

    return 0;
}
