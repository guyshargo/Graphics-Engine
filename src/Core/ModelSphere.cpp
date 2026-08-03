#include "ModelSphere.h"
#include "Utilities.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

ModelSphere::ModelSphere() 
    : center(0.0f, 0.0f, 0.0f), radius(0.0f), materialIndex(0), textureIndex(0) {}

ModelSphere::ModelSphere(glm::vec3 center, float radius, int materialIndex, int textureIndex)
    : center(center), radius(radius), materialIndex(materialIndex), textureIndex(textureIndex) {}

ModelSphere::ModelSphere(const std::string& toStringStr) {
    std::istringstream scanner(toStringStr);
    try {
        Utilities::parseTokenWithoutParameter(scanner, "Sphere");
        
        // Force strict evaluation order
        float cx = Utilities::parseTokenFloat(scanner, "center_x");
        float cy = Utilities::parseTokenFloat(scanner, "center_y");
        float cz = Utilities::parseTokenFloat(scanner, "center_z");
        
        center = glm::vec3(cx, cy, cz);
        
        radius = Utilities::parseTokenFloat(scanner, "radius");
        materialIndex = Utilities::parseTokenInt(scanner, "materialIndex");
        textureIndex = Utilities::parseTokenInt(scanner, "textureIndex");
        
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