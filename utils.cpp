#include "utils.hpp"

#include <sys/socket.h>  // for send()

#include <sstream>

#include "Server.hpp"

void parser(const std::string& input, std::vector<std::string>& output,
            char delimiter) {
    std::stringstream ss(input);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty())
            output.push_back(token);
    }
}

void sendError(Server& server, int clientFd, const std::string& errorCode,
               const std::string& nick, const std::string& details) {
    (void)server;  // currently unused
    sendError(clientFd, errorCode, nick, details);
}

void sendError(int clientFd, const std::string& errorCode,
               const std::string& nick, const std::string& details) {
    std::string msg =
        ":ft_irc " + errorCode + " " + nick + " " + details + "\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);
}

std::string trimWhitespace(const std::string& str) {
    std::string result = str;
    // Trim trailing whitespace
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' ||
                               result.back() == ' ' || result.back() == '\t')) {
        result.pop_back();
    }
    // Trim leading whitespace (if needed)
    size_t start = 0;
    while (start < result.length() &&
           (result[start] == ' ' || result[start] == '\t' ||
            result[start] == '\n' || result[start] == '\r')) {
        ++start;
    }
    return (start > 0) ? result.substr(start) : result;
}

std::string normalizeChannelName(const std::string& name) {
    std::string result = name;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}