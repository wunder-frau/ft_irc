#include "Client.hpp"

#include <sys/socket.h>
#include <unistd.h>  // For close()

#include <iostream>
#include <sstream>

Client::Client(int socket)
    : clientSocket(socket), authenticated(false), password("") {}

Client::Client(const Client &client)
    : clientSocket(client.clientSocket),
      nickName(client.nickName),
      userName(client.userName),
      hostName(client.hostName),
      realName(client.realName),
      authenticated(client.authenticated),
      buffer(client.buffer),
      password(client.password) {}

Client::~Client() {
    close(clientSocket);
}  // Close socket when client disconnects

std::string Client::getNickName() { return nickName; }
std::string Client::getUserName() { return userName; }
std::string Client::getBuffer() { return buffer; }
int Client::getSocket() { return clientSocket; }
std::string Client::getPassword() { return password; }
std::string Client::getHostname() { return hostName; }
bool Client::isAuthenticated() { return authenticated; }

void Client::setPassword(std::string &password) { this->password = password; }
void Client::setNickName(const std::string &nick) { nickName = nick; }
void Client::setUserName(const std::string &user) { userName = user; }
void Client::setHostName(const std::string &host) { hostName = host; }
void Client::setRealName(const std::string &real) { realName = real; }
void Client::authenticate() { authenticated = true; }

void Client::appendToBuffer(const std::string &data) { buffer += data; }
void Client::clearBuffer() { buffer.clear(); }

void Client::sendReply(int replyNumber, Client &client) {
    std::string message;

    switch (replyNumber) {
        case 001:
            message =
                "Welcome " + client.getNickName() + " to the IRC server!\r\n";
            break;
        case 002:
            message = "Your host is " + client.getHostname() + "\r\n";
            break;
        case 003:
            message = "This server was created recently!\r\n";
            break;
        case 464:
            message = "Password incorrect\r\n";
            break;
        default:
            return;
    }
    send(client.getSocket(), message.c_str(), message.length(), 0);
}

void Client::ERR_NICKNAMEINUSE(Client &client, const std::string &newNick) {
    std::stringstream ss;
    ss << ":server 433 " << client.getNickName() << " " << newNick
       << " :Nickname is already in use\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::handleUserInput() {
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input.empty())
            continue;

        // Ensure proper command formatting
        if (input == "/quit") {
            send(clientSocket, "QUIT :Goodbye!\r\n", 16, 0);
            break;
        } else if (input.rfind("JOIN ", 0) ==
                   0) {  // If input starts with "JOIN"
            std::string command = input + "\r\n";  // Ensure CRLF formatting
            send(clientSocket, command.c_str(), command.length(), 0);
            std::cout << "Sent: " << command;  // Debug output
        } else if (input.rfind("PRIVMSG ", 0) ==
                   0) {  // If input starts with "PRIVMSG"
            std::string command = input + "\r\n";  // Ensure CRLF formatting
            send(clientSocket, command.c_str(), command.length(), 0);
            std::cout << "Sent: " << command;
        } else {
            std::cout << "Unknown command: " << input << std::endl;
        }
    }
}

void Client::ERR_NOSUCHNICK(Client &client, const std::string &targetNick) {
    std::stringstream ss;
    ss << ":server 401 " << client.getNickName() << " " << targetNick
       << " :No such nick/channel\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::ERR_USERNOTINCHANNEL(Client &client, const std::string &targetNick,
                                  const std::string &channelName) {
    std::stringstream ss;
    ss << ":server 441 " << client.getNickName() << " " << targetNick << " "
       << channelName << " :They aren't on that channel\r\n";
    send(client.getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Client::broadcastModeChange(const std::string &setterNick, char mode,
                                 const std::string &targetNick,
                                 std::map<std::string, Client> &members,
                                 const std::string &channelName, char sign) {
    std::stringstream ss;
    ss << ":" << setterNick << " MODE " << channelName << " " << sign << mode
       << " " << targetNick << "\r\n";

    for (auto &member : members) {
        send(member.second.getSocket(), ss.str().c_str(), ss.str().length(), 0);
    }
}

void Client::handleServerResponse() {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            std::cout << "Disconnected from server.\n";
            break;
        }

        std::string msg(buffer);
        std::cout << "<Server>: " << msg << std::endl;

        // Handle PING from the server to prevent timeout
        if (msg.find("PING") == 0) {
            std::string pongResponse = "PONG " + msg.substr(5) + "\r\n";
            send(clientSocket, pongResponse.c_str(), pongResponse.length(), 0);
            std::cout << "Sent: " << pongResponse;
        }
    }
}
