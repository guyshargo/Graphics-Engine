#pragma once
#include <string>
#include <sstream>

namespace ParserUtils {

    /**
     * @brief Scans the configuration text file to verify that a specific category label is present 
     *        before the engine attempts to read the data underneath it.
     * 
     * @param scanner The input string stream being parsed.
     * @param parameterName The expected parameter name. The function expects this to be followed by a colon (e.g., "name:").
     * @throws std::runtime_error If the token is missing or does not match the expected format.
     */
    void parseTokenWithoutParameter(std::istringstream& scanner, const std::string& parameterName);

    /**
     * @brief Extracts a decimal number from the scene file to be used for 3D coordinates, 
     *        angles, or scaling factors in the engine.
     * 
     * @param scanner The input string stream being parsed.
     * @param parameterName The expected parameter name preceding the float.
     * @return The parsed float value.
     * @throws std::runtime_error If the float value is missing or invalid.
     */
    float parseTokenFloat(std::istringstream& scanner, const std::string& parameterName);

    /**
     * @brief Extracts a whole number from the scene file to set discrete engine parameters, 
     *        like the number of rays to cast, depth limits, or objects to load.
     * 
     * @param scanner The input string stream being parsed.
     * @param parameterName The expected parameter name preceding the integer.
     * @return The parsed integer value.
     * @throws std::runtime_error If the integer value is missing or invalid.
     */
    int parseTokenInt(std::istringstream& scanner, const std::string& parameterName);

    /**
     * @brief Extracts a single continuous word from the text file, typically used to retrieve 
     *        short names, texture formats, or simple file extensions.
     * 
     * @param scanner The input string stream being parsed.
     * @param parameterName The expected parameter name preceding the string.
     * @return The parsed string value.
     * @throws std::runtime_error If the string value is missing.
     */
    std::string parseTokenString(std::istringstream& scanner, const std::string& parameterName);

    /**
     * @brief Reads the entire remaining line of text, which allows the engine to safely load 
     *        file paths or texture names that contain blank spaces.
     * 
     * @param scanner The input string stream being parsed.
     * @param parameterName The expected parameter name preceding the string payload.
     * @return The remaining string, trimmed of leading and trailing whitespaces.
     */
    std::string parseTokenRestOfString(std::istringstream& scanner, const std::string& parameterName);
}