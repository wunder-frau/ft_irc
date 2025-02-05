#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <map>
#include <string>

class Client {
private:
    int clientSocket;
    std::string nickName;
    std::string userName;
    std::string hostName;
    std::string realName;
    bool authenticated;
    std::string buffer;
    std::string password;

public:
    Client(int socket);
    Client(const Client &client);
    ~Client();

    std::string getNickName();
    std::string getUserName();
    std::string getBuffer();
    int getSocket();
    std::string getPassword();
    std::string getHostname();
    bool isAuthenticated();

    void setPassword(std::string &password);
    void setNickName(const std::string &nick);
    void setUserName(const std::string &user);
    void setHostName(const std::string &host);
    void setRealName(const std::string &real);
    void authenticate();

    void appendToBuffer(const std::string &data);
    void clearBuffer();

    void sendReply(int replyNumber, Client &client);

    void ERR_NICKNAMEINUSE(Client &client, const std::string &newNick);
    void ERR_NOSUCHNICK(Client &client, const std::string &targetNick);
    void ERR_USERNOTINCHANNEL(Client &client, const std::string &targetNick,
                              const std::string &channelName);

    void RPL_PUBLICCHANNEL(Client &client, const std::string &channelName,
                           const bool &inviteOnly);
    void RPL_TOPIC(Client &client, const std::string &channelName,
                   const std::string &topic);

    void broadcastModeChange(const std::string &setterNick, char mode,
                             const std::string &targetNick,
                             std::map<std::string, Client> &members,
                             const std::string &channelName, char sign);
};

#endif
