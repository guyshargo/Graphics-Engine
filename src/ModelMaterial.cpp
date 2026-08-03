#include "ModelMaterial.h"
#include "Utilities.h"
#include <sstream>
#include <stdexcept>
#include <iostream>

ModelMaterial::ModelMaterial()
    : kColor(0.0f), color(0.0f), kDirect(0.0f), ka(0.0f), kd(0.0f), ks(0.0f),
      shininess(1.0f), kReflection(0.0f), kTransmission(0.0f), refractiveIndex(1.0f),
      kTexture(0.0f), comment("") {}

ModelMaterial::ModelMaterial(const std::string& toStringStr) {
    std::istringstream scanner(toStringStr);
    try {
        Utilities::parseTokenWithoutParameter(scanner, "Material");
        kColor = Utilities::parseTokenFloat(scanner, "kColor");
        
        color = glm::vec3(
            Utilities::parseTokenFloat(scanner, "color_R"),
            Utilities::parseTokenFloat(scanner, "color_G"),
            Utilities::parseTokenFloat(scanner, "color_B")
        );
        
        kDirect = Utilities::parseTokenFloat(scanner, "kDirect");
        
        ka = glm::vec3(
            Utilities::parseTokenFloat(scanner, "ka_R"),
            Utilities::parseTokenFloat(scanner, "ka_G"),
            Utilities::parseTokenFloat(scanner, "ka_B")
        );
        
        kd = glm::vec3(
            Utilities::parseTokenFloat(scanner, "kd_R"),
            Utilities::parseTokenFloat(scanner, "kd_G"),
            Utilities::parseTokenFloat(scanner, "kd_B")
        );
        
        ks = glm::vec3(
            Utilities::parseTokenFloat(scanner, "ks_R"),
            Utilities::parseTokenFloat(scanner, "ks_G"),
            Utilities::parseTokenFloat(scanner, "ks_B")
        );
        
        shininess = Utilities::parseTokenFloat(scanner, "shininess");
        kReflection = Utilities::parseTokenFloat(scanner, "kReflection");
        kTransmission = Utilities::parseTokenFloat(scanner, "kTransmission");
        refractiveIndex = Utilities::parseTokenFloat(scanner, "refractiveIndex");
        kTexture = Utilities::parseTokenFloat(scanner, "kTexture");
        comment = Utilities::parseTokenRestOfString(scanner, "comment");

    } catch (const std::exception& e) {
        std::string errorMessage = std::string(e.what()) + 
                                   "\nAt Material constructor from toStringStr.\n Fail to load parse string:\n" + 
                                   toStringStr;
        std::cerr << errorMessage << std::endl;
        throw std::runtime_error(errorMessage);
    }
}

std::string ModelMaterial::toString() const {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
        "Material: kColor: %f color_R: %f color_G: %f color_B: %f kDirect: %f ka_R: %f ka_G: %f ka_B: %f kd_R: %f kd_G: %f kd_B: %f ks_R: %f ks_G: %f ks_B: %f shininess: %f kReflection: %f kTransmission: %f refractiveIndex: %f kTexture: %f comment: %s",
        kColor, color.x, color.y, color.z, kDirect, ka.x, ka.y, ka.z, kd.x, kd.y, kd.z, ks.x, ks.y, ks.z, shininess, kReflection, kTransmission, refractiveIndex, kTexture, comment.c_str());
    return std::string(buffer);
}