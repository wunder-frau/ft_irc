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
        client.setUsername("testuser");

        server.addClient(client);
        std::string nickCommand = "NICK NewNick";
        server.nick(1, nickCommand);

        Client *updatedClient = server.getClientObjByFd(1);
        if (updatedClient) {
            std::cout << "Updated Nickname: " << updatedClient->getNick()
                      << std::endl;
        } else {
            std::cout << "Client not found." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
