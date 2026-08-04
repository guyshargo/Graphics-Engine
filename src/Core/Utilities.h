#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <filesystem>

namespace Utilities {

    // ==========================================
    // Ray Tracer: Text Parsing Utilities
    // ==========================================
    void parseTokenWithoutParameter(std::istringstream& scanner, const std::string& parameterName);
    float parseTokenFloat(std::istringstream& scanner, const std::string& parameterName);
    int parseTokenInt(std::istringstream& scanner, const std::string& parameterName);
    std::string parseTokenString(std::istringstream& scanner, const std::string& parameterName);
    std::string parseTokenRestOfString(std::istringstream& scanner, const std::string& parameterName);

    // ==========================================
    // Rasterizer: File & Image Utilities
    // ==========================================
    std::string openFileChooser(const std::string& fileExtension, const std::string& initialDirectory);
    std::string saveFileChooser(const std::string& fileExtension, const std::string& initialDirectory);
    std::string getRelativePath(const std::string& absolutePath);
    void saveIntBufferAsBMP(const std::vector<int>& buffer, int width, int height, const std::string& filePath);
    void saveIntBufferAsCSV(const std::vector<int>& buffer, int width, int height, const std::string& filePath);
}