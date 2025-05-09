#pragma once

#include <string>
#include <vector>

class Server;

/// Splits a string by a delimiter and fills a vector with the parts.
void parser(const std::string& input, std::vector<std::string>& output, char delimiter);

/// Sends an IRC error with optional Server object (used if needed for future
/// logging, etc.)
void sendError(Server& server, int clientFd, const std::string& errorCode, const std::string& nick,
               const std::string& details);

/// Sends an IRC error without requiring a Server instance.
void sendError(int clientFd, const std::string& errorCode, const std::string& nick,
               const std::string& details);

/// Trims whitespace characters (space, tab, newline, carriage return) from both
/// ends of a string.
std::string trimWhitespace(const std::string& str);

std::string toUpperCase(const std::string& str);
std::string normalizeChannelName(const std::string& name);
std::string ircCaseFold(const std::string& input);
