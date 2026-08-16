#include <sstream>
#include <stdexcept>
#include <iostream>

#include "./Scene/ModelSphere.h"
#include "../Utilities/ParserUtils.h"

ModelSphere::ModelSphere() 
    : center(0.0f, 0.0f, 0.0f), radius(0.0f), materialIndex(0), textureIndex(0) {}

ModelSphere::ModelSphere(glm::vec3 center, float radius, int materialIndex, int textureIndex)
    : center(center), radius(radius), materialIndex(materialIndex), textureIndex(textureIndex) {}

ModelSphere::ModelSphere(const std::string& toStringStr) {
    std::istringstream scanner(toStringStr);
    try {
        ParserUtils::parseTokenWithoutParameter(scanner, "Sphere");
        
        // Force strict evaluation order
        float cx = ParserUtils::parseTokenFloat(scanner, "center_x");
        float cy = ParserUtils::parseTokenFloat(scanner, "center_y");
        float cz = ParserUtils::parseTokenFloat(scanner, "center_z");
        
        center = glm::vec3(cx, cy, cz);
        
        radius = ParserUtils::parseTokenFloat(scanner, "radius");
        materialIndex = ParserUtils::parseTokenInt(scanner, "materialIndex");
        textureIndex = ParserUtils::parseTokenInt(scanner, "textureIndex");
        
    } catch (const std::exception& e) {
        std::string errorMessage = std::string(e.what()) + 
                                   "\nAt Sphere constructor from toStringStr.\n Fail to load parse string:\n" + 
                                   toStringStr;
        std::cerr << errorMessage << std::endl;
        throw std::runtime_error(errorMessage);
    }
}

std::string ModelSphere::toString() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), 
             "Sphere: center_x: %f center_y: %f center_z: %f radius: %f materialIndex: %d textureIndex: %d",
             center.x, center.y, center.z, radius, materialIndex, textureIndex);
    return std::string(buffer);
}