#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <filesystem>
#include <glm/glm.hpp>

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

    // ==========================================
    // Shared memory writing logic
    // ==========================================
    void WriteColorToBuffer(std::vector<uint32_t>& buffer, int x, int y, const glm::vec3& color, int imageWidth, int imageHeight);

    // ==========================================
    // Math Utilities
    // ==========================================
    glm::vec3 lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, 
                               const glm::vec3& Kd, const glm::vec3& Ks, const glm::vec3& Ka, float shininess);

    // Overload for uniform float parameters
    float lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, 
                           float Kd, float Ks, float Ka, float shininess);
}