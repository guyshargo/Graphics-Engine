#pragma once
#include <string>
#include <sstream>

namespace ParserUtils {
    void parseTokenWithoutParameter(std::istringstream& scanner, const std::string& parameterName);
    float parseTokenFloat(std::istringstream& scanner, const std::string& parameterName);
    int parseTokenInt(std::istringstream& scanner, const std::string& parameterName);
    std::string parseTokenString(std::istringstream& scanner, const std::string& parameterName);
    std::string parseTokenRestOfString(std::istringstream& scanner, const std::string& parameterName);
}