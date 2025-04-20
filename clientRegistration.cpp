#include <iostream>
#include <regex>

#include "Server.hpp"
#include "regexRules.hpp"
#include "utils.hpp"  // for parser

// Validate PASS command: extract and compare password
void Server::registerPassword(Client& client, const std::string& arg,
                              size_t* clientIndex) {
    std::size_t found = arg.find("PASS");
    if (found != std::string::npos && arg.length() > 5) {
        std::string pwd = arg.substr(found + 5);

        // 💡 Удаляем пробелы, \r, \n по краям
        pwd.erase(0, pwd.find_first_not_of(" \r\n"));
        pwd.erase(pwd.find_last_not_of(" \r\n") + 1);

        if (pwd == _password) {
            std::string msg = "Password accepted.\r\n";
            send(client.getFd(), msg.c_str(), msg.length(), 0);
            client.setPassword(pwd);
        } else {
            std::string msg =
                "ERROR :Incorrect password. Connection closed.\r\n";
            send(client.getFd(), msg.c_str(), msg.length(), 0);
            std::cerr << "[WARN] Incorrect password from client fd="
                      << client.getFd() << std::endl;
            eraseClient(client.getFd(), clientIndex);
        }
    }
}

// Validate NICK command: extract, check uniqueness, set nickname
void Server::registerNickname(Client& client, const std::string& arg) {
    std::size_t found = arg.find("NICK");
    if (found != std::string::npos && arg.length() > 5) {
        if (client.getPassword().empty()) {
            std::string msg = "ERROR :Please enter PASS before NICK\r\n";
            send(client.getFd(), msg.c_str(), msg.length(), 0);
            return;
        }

        std::string nick = arg.substr(found + 5);
        if (std::regex_match(nick, incorrectRegex)) {
            std::cerr << "[WARN] Rejected nick with invalid pattern from fd="
                      << client.getFd() << std::endl;
            return;
        }

        int nickCount = 1;
        std::string newNick = nick;
        while (!isUniqueNick(newNick)) {
            newNick = nick + std::to_string(nickCount++);
        }

        client.setNickname(newNick);
    }
}

// Validate USER command: extract and assign username
void Server::registerUser(Client& client, const std::string& arg) {
    std::size_t found = arg.find("USER");
    if (found != std::string::npos && arg.length() > 5) {
        if (client.getPassword().empty()) {
            std::string msg = "ERROR :Please enter PASS before USER\r\n";
            send(client.getFd(), msg.c_str(), msg.length(), 0);
            return;
        }

        std::vector<std::string> params;
        parser(arg, params, ' ');

        if (params.size() >= 2) {
            std::string username = params.at(1);
            if (std::regex_match(username, incorrectRegex)) {
                std::cerr
                    << "[WARN] Rejected username with invalid pattern from fd="
                    << client.getFd() << std::endl;
                return;
            }
            client.setUsername(username);
        }
    }
}

// Final authentication: if all fields are set, welcome the client
void Server::authenticate(Client& client, const std::string& arg,
                          size_t* clientIndex) {
    if (client.getPassword().empty())
        registerPassword(client, arg, clientIndex);
    if (client.getNick().empty())
        registerNickname(client, arg);
    if (client.getUser().empty())
        registerUser(client, arg);

    if (!client.getPassword().empty() && !client.getNick().empty() &&
        !client.getUser().empty()) {
        std::string msg = ":ft_irc 001 " + client.getNick() +
                          " :Welcome to the IRC Network, " + client.getNick() +
                          "!\r\n";
        send(client.getFd(), msg.c_str(), msg.length(), 0);

        msg = ":ft_irc 002 " + client.getNick() +
              " :Your host is ft_irc, running version 42\r\n";
        send(client.getFd(), msg.c_str(), msg.length(), 0);

        msg = ":ft_irc 005 " + client.getNick() +
              " INVITE MODE JOIN KICK TOPIC MSG NICK QUIT :are supported by "
              "this server\r\n";
        send(client.getFd(), msg.c_str(), msg.length(), 0);

        client.setAsRegistered();
        std::cout << "[INFO] Client fd=" << client.getFd()
                  << " successfully authenticated." << std::endl;
    }
}

// Entry point: find client by fd and process registration
void Server::registerClient(int clientFd, const std::string& arg,
                            size_t* clientIndex) {
    for (std::deque<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it) {
        if (it->getFd() == clientFd) {
            authenticate(*it, arg, clientIndex);
            return;
        }
    }
}
