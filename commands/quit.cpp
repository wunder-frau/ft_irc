#include "../Server.hpp"
#include "../utils.hpp"
#include <iostream>
#include <unistd.h>  // close()

void executeQuit(Server& server, int clientFd, const std::string& arg)
{
    Client* client = server.getClientObjByFd(clientFd);
    if (!client)
        return;

    std::string quitMsg = arg;
    std::string reason = "Client Quit";

    size_t colonPos = quitMsg.find(':');
    if (colonPos != std::string::npos)
        reason = trimWhitespace(quitMsg.substr(colonPos + 1));

    std::string message = ":" + client->getNick() + "!~" + client->getUser() + "@" +
                          client->getIPa() + " QUIT :" + reason + "\r\n";
    std::cout << "[QUIT] " << client->getNick() << " has quit: " << reason << std::endl;
    
    // Broadcast to all channels the client was in
    for (std::vector<Channel>::iterator it = server.getChannels().begin();
         it != server.getChannels().end(); ++it)
    {
        if (it->isInChannel(client))
        {
            it->broadcast(message, client);
        }
    }
    
    // Clean up
    try
    {
        size_t clientIndex = server.getClientIndex(clientFd);
        server.handleClientDisconnect(clientFd, &clientIndex);  // Safely remove client
    }
    catch (const std::exception& e)
    {
        std::cerr << "[QUIT] Error removing client: " << e.what() << std::endl;
        close(clientFd);  // Just in case socket wasn't closed
    }
}
