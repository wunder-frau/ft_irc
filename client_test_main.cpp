#include <iostream>

#include "Client.hpp"

int main() {
    Client testClient(42, "127.0.0.1");

    testClient.setNickname("CoolUser");
    testClient.setUsername("coolguy");

    std::cout << "FD: " << testClient.getFd() << std::endl;
    std::cout << "Nickname: " << testClient.getNick() << std::endl;
    std::cout << "Username: " << testClient.getUser() << std::endl;
    std::cout << "IP: " << testClient.getIPa() << std::endl;
    std::cout << "Is Registered? " << (testClient.isRegistered() ? "Yes" : "No")
              << std::endl;

    testClient.setAsRegistered();
    std::cout << "Now Registered? "
              << (testClient.isRegistered() ? "Yes" : "No") << std::endl;

    return 0;
}
