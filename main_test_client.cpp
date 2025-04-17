#include <iostream>

#include "Client.hpp"

int main() {
    Client client1(5, "192.168.0.1");

    client1.setNickname("Bob");
    client1.setPassword("pass123");
    client1.setUsername("bobUser");
    client1.setAsRegistered();

    std::cout << "Client1 Details:" << std::endl;
    std::cout << "FD: " << client1.getFd() << std::endl;
    std::cout << "IP Address: " << client1.getIPa() << std::endl;
    std::cout << "Nickname: " << client1.getNick() << std::endl;
    std::cout << "Password: " << client1.getPassword() << std::endl;
    std::cout << "Username: " << client1.getUser() << std::endl;
    std::cout << "Registered: " << (client1.isRegistered() ? "Yes" : "No")
              << std::endl;

    Client client2(client1);
    std::cout << "\nCopied Client (client2) Details:" << std::endl;
    std::cout << "FD: " << client2.getFd() << std::endl;
    std::cout << "IP Address: " << client2.getIPa() << std::endl;
    std::cout << "Nickname: " << client2.getNick() << std::endl;
    std::cout << "Password: " << client2.getPassword() << std::endl;
    std::cout << "Username: " << client2.getUser() << std::endl;
    std::cout << "Registered: " << (client2.isRegistered() ? "Yes" : "No")
              << std::endl;

    Client client3;
    client3 = client1;
    std::cout << "\nAssigned Client (client3) Details:" << std::endl;
    std::cout << "FD: " << client3.getFd() << std::endl;
    std::cout << "IP Address: " << client3.getIPa() << std::endl;
    std::cout << "Nickname: " << client3.getNick() << std::endl;
    std::cout << "Password: " << client3.getPassword() << std::endl;
    std::cout << "Username: " << client3.getUser() << std::endl;
    std::cout << "Registered: " << (client3.isRegistered() ? "Yes" : "No")
              << std::endl;

    return 0;
}
