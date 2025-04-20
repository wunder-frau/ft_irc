// main_test_nick.cpp
#include <iostream>
#include <stdexcept>
#include <string>

#include "Client.hpp"
#include "Server.hpp"

int main() {
    try {
        Server server(6667, "secret");
        Client client(1, "127.0.0.1");

        client.setNickname("OldNick");
        client.setUsername("testUser");

        server.addClient(client);

        // Simulate a NICK command; adjust the command string if needed.
        std::string nickCommand = "NICK NewNick";
        server.dispatchCommand(nickCommand, 1);

        Client* updatedClient = server.getClientObjByFd(1);
        if (updatedClient)
            std::cout << "Updated Nickname: " << updatedClient->getNick() << std::endl;
        else
            std::cout << "Client not found." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
