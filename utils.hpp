#pragma once

#include <string>
#include <vector>
#include <sstream>

// Utility function to split a string by a delimiter
inline void splitTokens(const std::string& input, std::vector<std::string>& output, char delim)
{
    std::stringstream ss(input);
    std::string token;
    while (std::getline(ss, token, delim))
    {
        if (!token.empty())
            output.push_back(token);
    }
}
