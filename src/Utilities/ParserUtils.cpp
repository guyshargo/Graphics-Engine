#include "ParserUtils.h"
#include <iostream>
#include <stdexcept>

namespace ParserUtils {
    void parseTokenWithoutParameter(std::istringstream& scanner, const std::string& parameterName) {
        std::string token;
        if (!(scanner >> token)) {
            std::string errorStr = "At parseTokenWithoutParameter, Parsing " + parameterName + ", token is missing.";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        
        std::string expectedFormat = parameterName + ":";
        if (token != expectedFormat) {
            std::string errorStr = "At parseTokenWithoutParameter, Parsing " + parameterName + ", the token " + expectedFormat + " is missing. Found: " + token;
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
    }

    float parseTokenFloat(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        float value;
        if (!(scanner >> value)) {
            std::string errorStr = "At parseTokenFloat, parsing " + parameterName + ", float is missing after token " + parameterName + ":";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        return value;
    }

    int parseTokenInt(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        int value;
        if (!(scanner >> value)) {
            std::string errorStr = "At parseTokenInt, parsing " + parameterName + ", int is missing after token " + parameterName + ":";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        return value;
    }

    std::string parseTokenString(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        std::string value;
        if (!(scanner >> value)) {
            std::string errorStr = "At parseTokenString, parsing " + parameterName + ", string is missing after token " + parameterName + ":";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        return value;
    }

    std::string parseTokenRestOfString(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        std::string restOfLine;
        // Read the rest of the string
        std::getline(scanner, restOfLine);
        
        // Trim leading and trailing whitespaces
        size_t start = restOfLine.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return ""; 
        size_t end = restOfLine.find_last_not_of(" \t\r\n");
        
        return restOfLine.substr(start, end - start + 1);
    }
}