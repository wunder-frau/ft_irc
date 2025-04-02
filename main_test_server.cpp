#include <iostream>

#include "Server.hpp"

int main() {
    std::cout << "[Step 1] Creating Server instance on port 6667 with password "
                 "'secret'.\n";
    Server server(6667, "secret");

    std::cout
        << "[Step 2] Creating Client instance with FD 10 and IP '127.0.0.1'.\n";
    Client client(10, "127.0.0.1");

    std::cout << "[Step 3] Adding client to server's client list.\n";
    server.addClient(client);

    size_t clientIndex = 0;
    int fd = 10;

    std::cout << "[Step 4a] Processing PASS command: 'PASS secret'\n";
    server.registerClient(fd, "PASS secret", &clientIndex);

    std::cout << "[Step 4b] Processing NICK command: 'NICK Alice'\n";
    server.registerClient(fd, "NICK Alice_lol_kek", &clientIndex);

    std::cout << "[Step 4c] Processing USER command: 'USER alice 0 * :Alice'\n";
    server.registerClient(fd, "USER alice 0 * :Alice", &clientIndex);

    std::cout << "[Step 5] Checking registration status...\n";
    bool regStatus = server.isRegistered(fd);
    std::cout << "Client registration status: "
              << (regStatus ? "Registered" : "Not Registered") << std::endl;

    // Retrieve the updated client from the server:
    Client* updatedClient = server.getClientObjByFd(fd);
    if (updatedClient) {
        std::cout << "\n[Updated Client Details]" << std::endl;
        std::cout << "FD: " << updatedClient->getFd() << std::endl;
        std::cout << "IP Address: " << updatedClient->getIPa() << std::endl;
        std::cout << "Nickname: " << updatedClient->getNick() << std::endl;
        std::cout << "Username: " << updatedClient->getUser() << std::endl;
        std::cout << "Password: " << updatedClient->getPassword() << std::endl;
        std::cout << "Registered: "
                  << (updatedClient->isRegistered() ? "Yes" : "No")
                  << std::endl;
    } else {
        std::cout << "Client not found in the server's list." << std::endl;
    }

    return 0;
}
