#include "ParserUtils.h"
#include <iostream>
#include <stdexcept>

namespace ParserUtils {
    void parseTokenWithoutParameter(std::istringstream& scanner, const std::string& parameterName) {
        std::string token;

        // Attempt to extract the next block of text from the stream
        if (!(scanner >> token)) {
            std::string errorStr = "At parseTokenWithoutParameter, Parsing " + parameterName + ", token is missing.";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        
        // Validate that the token strictly matches the "parameter:" format
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
        // Extract the raw numeric payload into a float
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
        // Extract the raw numeric payload into an integer
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
        // Extract the next continuous string (stops at whitespace)
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
        // Consume the remainder of the stream into a single string, including spaces
        std::getline(scanner, restOfLine);
        
        // Trim leading whitespaces
        size_t start = restOfLine.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";

        // Trim trailing whitespaces
        size_t end = restOfLine.find_last_not_of(" \t\r\n");
        
        return restOfLine.substr(start, end - start + 1);
    }
}