#include "ModelLight.h"
#include "Utilities.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

ModelLight::ModelLight() 
    : location(0.0f, 0.0f, 0.0f), intensity(1.0f), comment("") {}

ModelLight::ModelLight(glm::vec3 location, float intensity, const std::string& comment)
    : location(location), intensity(intensity), comment(comment) {}

ModelLight::ModelLight(const std::string& toStringStr) {
    std::istringstream scanner(toStringStr);
    try {
        Utilities::parseTokenWithoutParameter(scanner, "Light");
        
        // Force strict evaluation order
        float locX = Utilities::parseTokenFloat(scanner, "location_x");
        float locY = Utilities::parseTokenFloat(scanner, "location_y");
        float locZ = Utilities::parseTokenFloat(scanner, "location_z");
        
        location = glm::vec3(locX, locY, locZ);
        
        intensity = Utilities::parseTokenFloat(scanner, "intensity");
        comment = Utilities::parseTokenRestOfString(scanner, "comment");
        
    } catch (const std::exception& e) {
        std::string errorMessage = std::string(e.what()) + 
                                   "\nAt Light constructor from toStringStr.\n Fail to load parse string:\n" + 
                                   toStringStr;
        std::cerr << errorMessage << std::endl;
        throw std::runtime_error(errorMessage);
    }
}

std::string ModelLight::toString() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), 
             "Light: location_x: %f location_y: %f location_z: %f intensity: %f comment: %s",
             location.x, location.y, location.z, intensity, comment.c_str());
    return std::string(buffer);
}