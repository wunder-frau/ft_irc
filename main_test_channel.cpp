#include <iostream>
#include "Server.hpp"
#include "Channel.hpp"

int main() {
    std::cout << "[TEST] IRC Channel Functionality\n";
    
    // Create server instance
    Server server(6667, "secret");
    
    // Create test clients
    Client alice(1, "127.0.0.1");
    alice.setNickname("Alice");
    alice.setUsername("alice");
    alice.setAsRegistered();
    
    Client bob(2, "127.0.0.1");
    bob.setNickname("Bob");
    bob.setUsername("bob");
    bob.setAsRegistered();
    
    Client charlie(3, "127.0.0.1");
    charlie.setNickname("Charlie");
    charlie.setUsername("charlie");
    charlie.setAsRegistered();
    
    // Add clients to server
    server.addClient(alice);
    server.addClient(bob);
    server.addClient(charlie);
    
    std::cout << "\n[Step 1] Testing channel creation (JOIN command)\n";
    server.handleJoin(1, "JOIN #testchannel");
    
    // Get the created channel
    Channel* channel = server.findChannel("#testchannel");
    if (channel) {
        std::cout << "Channel #testchannel created successfully.\n";
        std::cout << "Channel operator: " << channel->getClients()[0]->getNick() << "\n";
    } else {
        std::cout << "Failed to create channel.\n";
        return 1;
    }
    
    std::cout << "\n[Step 2] Testing JOIN for another user\n";
    server.handleJoin(2, "JOIN #testchannel");
    
    std::cout << "\n[Step 3] Testing channel TOPIC command\n";
    server.handleTopic(1, "TOPIC #testchannel Welcome to the test channel!");
    std::cout << "Channel topic: " << channel->getTopic() << "\n";
    
    std::cout << "\n[Step 4] Testing MODE command (set invite-only)\n";
    server.handleMode(1, "MODE #testchannel +i");
    std::cout << "Channel is invite-only: " << (channel->isInviteOnly() ? "Yes" : "No") << "\n";
    
    std::cout << "\n[Step 5] Testing INVITE command\n";
    server.handleInvite(1, "INVITE Charlie #testchannel");
    std::cout << "Charlie is invited: " << (channel->isInvited("Charlie") ? "Yes" : "No") << "\n";
    
    std::cout << "\n[Step 6] Invited user joining channel\n";
    server.handleJoin(3, "JOIN #testchannel");
    
    std::cout << "\n[Step 7] List channel members\n";
    std::cout << "Channel members:\n";
    for (Client* client : channel->getClients()) {
        std::cout << "- " << client->getNick() << (channel->isOperator(client) ? " (operator)" : "") << "\n";
    }
    
    std::cout << "\n[Step 8] Testing KICK command\n";
    server.handleKick(1, "KICK #testchannel Bob Goodbye Bob!");
    
    std::cout << "\nChannel members after kick:\n";
    for (Client* client : channel->getClients()) {
        std::cout << "- " << client->getNick() << (channel->isOperator(client) ? " (operator)" : "") << "\n";
    }
    
    std::cout << "\n[Step 9] Testing PART command\n";
    server.handlePart(3, "PART #testchannel Leaving the channel");
    
    std::cout << "\nChannel members after part:\n";
    for (Client* client : channel->getClients()) {
        std::cout << "- " << client->getNick() << (channel->isOperator(client) ? " (operator)" : "") << "\n";
    }
    
    std::cout << "\n[Step 10] Testing channel removal when empty\n";
    server.handlePart(1, "PART #testchannel Closing the channel");
    
    // Check if channel was removed
    if (server.findChannel("#testchannel") == nullptr) {
        std::cout << "Channel was automatically removed when empty.\n";
    } else {
        std::cout << "Channel still exists after all users left.\n";
        server.removeEmptyChannels();
        if (server.findChannel("#testchannel") == nullptr) {
            std::cout << "Channel removed after explicit cleanup.\n";
        }
    }
    
    std::cout << "\n[TEST COMPLETE] Channel functionality test finished.\n";
    return 0;
} 