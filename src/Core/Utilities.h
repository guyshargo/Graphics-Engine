#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace Utilities {
    
    // Equivalent to Utilities.parseTokenWithoutParameter
    void parseTokenWithoutParameter(std::istringstream& scanner, const std::string& parameterName);

    // Equivalent to Utilities.parseTokenFloat
    float parseTokenFloat(std::istringstream& scanner, const std::string& parameterName);

    // Equivalent to Utilities.parseTokenInt
    int parseTokenInt(std::istringstream& scanner, const std::string& parameterName);

    // Equivalent to Utilities.parseTokenString
    std::string parseTokenString(std::istringstream& scanner, const std::string& parameterName);

    // Equivalent to Utilities.parseTokenRestOfString
    std::string parseTokenRestOfString(std::istringstream& scanner, const std::string& parameterName);

}