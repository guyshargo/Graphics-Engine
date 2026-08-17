#include <sstream>
#include <stdexcept>
#include <iostream>

#include "ModelLight.h"
#include "../Utilities/ParserUtils.h"

ModelLight::ModelLight() 
    : location(0.0f, 0.0f, 0.0f), intensity(1.0f), radius(0.0f), comment("") {}

ModelLight::ModelLight(glm::vec3 location, float intensity, float radius, const std::string& comment)
    : location(location), intensity(intensity), radius(radius), comment(comment) {}

ModelLight::ModelLight(const std::string& toStringStr) {
    std::istringstream scanner(toStringStr);
    try {
        ParserUtils::parseTokenWithoutParameter(scanner, "Light");
        
        // Extract 3D position coordinates in a strict order to prevent stream desynchronization
        float locX = ParserUtils::parseTokenFloat(scanner, "location_x");
        float locY = ParserUtils::parseTokenFloat(scanner, "location_y");
        float locZ = ParserUtils::parseTokenFloat(scanner, "location_z");
        
        location = glm::vec3(locX, locY, locZ);
        
        intensity = ParserUtils::parseTokenFloat(scanner, "intensity");

        // Accommodate older scene files that might not include a soft shadow radius
        if (toStringStr.find("radius:") != std::string::npos) {
            radius = ParserUtils::parseTokenFloat(scanner, "radius");
        } else {
            radius = 2.0f; // Default radius fallback
        }
        
        // Consume the rest of the line as an optional description
        comment = ParserUtils::parseTokenRestOfString(scanner, "comment");
        
    } catch (const std::exception& e) {
        std::string errorMessage = std::string(e.what()) + 
                                   "\nAt Light constructor from toStringStr.\n Fail to load parse string:\n" + 
                                   toStringStr;
        std::cerr << errorMessage << std::endl;
        throw std::runtime_error(errorMessage);
    }
}
