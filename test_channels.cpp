#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"

// Dummy send() implementation for testing purposes.
// The 'flags' parameter is unnamed to avoid unused parameter warnings.
ssize_t send(int socket, const void* buffer, size_t length, int /*flags*/) {
    std::cout << "[Socket " << socket << "] "
              << std::string(static_cast<const char*>(buffer), length);
    return length;
}

void printHeader(const std::string& title) {
    std::cout << "\n\n" << std::string(80, '=') << std::endl;
    std::cout << "   " << title << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

void printChannelMembers(Channel* channel) {
    if (!channel) {
        std::cout << "Channel not found!" << std::endl;
        return;
    }
    
    std::cout << "Channel: " << channel->getName() << std::endl;
    std::cout << "Topic: " << (channel->getTopic().empty() ? "<no topic>" : channel->getTopic()) << std::endl;
    std::cout << "Invite-only: " << (channel->isInviteOnly() ? "Yes" : "No") << std::endl;
    std::cout << "Members:" << std::endl;
    
    std::vector<Client*> members = channel->getClients();
    for (size_t i = 0; i < members.size(); ++i) {
        std::cout << "  " << (i+1) << ". " << members[i]->getNick() 
                  << (channel->isOperator(members[i]) ? " (operator)" : "") 
                  << " [FD: " << members[i]->getFd() << "]" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    try {
        // Create a Server instance
        Server server(6667, "secret");
        
        // Create test clients
        Client alice(101, "192.168.1.101");
        alice.setNickname("Alice");
        alice.setUsername("alice_user");
        alice.setPassword("secret");
        alice.setAsRegistered();
        
        Client bob(102, "192.168.1.102");
        bob.setNickname("Bob");
        bob.setUsername("bob_user");
        bob.setPassword("secret");
        bob.setAsRegistered();
        
        Client charlie(103, "192.168.1.103");
        charlie.setNickname("Charlie");
        charlie.setUsername("charlie_user");
        charlie.setPassword("secret");
        charlie.setAsRegistered();
        
        // Add clients to server
        server.addClient(alice);
        server.addClient(bob);
        server.addClient(charlie);
        
        // TEST 1: Channel Creation and Joining
        printHeader("TEST 1: Channel Creation and Joining");
        std::cout << "Alice creates and joins #general" << std::endl;
        server.handleJoin(101, "JOIN #general");
        
        Channel* general = server.findChannel("#general");
        printChannelMembers(general);
        
        std::cout << "Bob joins #general" << std::endl;
        server.handleJoin(102, "JOIN #general");
        printChannelMembers(general);
        
        // TEST 2: Topic Setting
        printHeader("TEST 2: Topic Setting");
        std::cout << "Alice (operator) sets the topic" << std::endl;
        server.handleTopic(101, "TOPIC #general Welcome to the general channel!");
        printChannelMembers(general);
        
        // TEST 3: Invite-Only Mode
        printHeader("TEST 3: Invite-Only Mode");
        std::cout << "Alice sets the channel to invite-only" << std::endl;
        server.handleMode(101, "MODE #general +i");
        printChannelMembers(general);
        
        std::cout << "Charlie tries to join without invitation (should fail)" << std::endl;
        server.handleJoin(103, "JOIN #general");
        printChannelMembers(general);
        
        std::cout << "Alice invites Charlie" << std::endl;
        server.handleInvite(101, "INVITE Charlie #general");
        
        std::cout << "Charlie joins after invitation" << std::endl;
        server.handleJoin(103, "JOIN #general");
        printChannelMembers(general);
        
        // TEST 4: Kicking a User
        printHeader("TEST 4: Kicking a User");
        std::cout << "Alice kicks Bob" << std::endl;
        server.handleKick(101, "KICK #general Bob Goodbye Bob!");
        printChannelMembers(general);
        
        // TEST 5: Part Channel
        printHeader("TEST 5: Part Channel");
        std::cout << "Charlie parts from the channel" << std::endl;
        server.handlePart(103, "PART #general I'm leaving!");
        printChannelMembers(general);
        
        // TEST 6: Create a Second Channel
        printHeader("TEST 6: Create a Second Channel");
        std::cout << "Bob creates and joins #random" << std::endl;
        server.handleJoin(102, "JOIN #random");
        
        Channel* random = server.findChannel("#random");
        printChannelMembers(random);
        
        // Print summary
        printHeader("CHANNEL SUMMARY");
        std::cout << "Channels on the server:" << std::endl;
        
        if (server.findChannel("#general")) {
            std::cout << "- #general exists" << std::endl;
            printChannelMembers(server.findChannel("#general"));
        }
        
        if (server.findChannel("#random")) {
            std::cout << "- #random exists" << std::endl;
            printChannelMembers(server.findChannel("#random"));
        }
        
        // TEST 7: Channel Cleanup
        printHeader("TEST 7: Channel Cleanup");
        std::cout << "Alice leaves #general (should be removed as it's now empty)" << std::endl;
        server.handlePart(101, "PART #general Goodbye!");
        
        std::cout << "Checking if #general still exists..." << std::endl;
        if (server.findChannel("#general")) {
            std::cout << "- #general still exists!" << std::endl;
        } else {
            std::cout << "- #general has been removed (as expected)" << std::endl;
        }
        
        if (server.findChannel("#random")) {
            std::cout << "- #random still exists" << std::endl;
            printChannelMembers(server.findChannel("#random"));
        }
        
        std::cout << "\nTest completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 