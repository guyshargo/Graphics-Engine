#include <sstream>
#include <stdexcept>
#include <iostream>
#include "ModelMaterial.h"
#include "../Utilities/ParserUtils.h"

ModelMaterial::ModelMaterial()
    : kColor(0.0f), color(0.0f), kDirect(0.0f), ka(0.0f), kd(0.0f), ks(0.0f),
      shininess(1.0f), kReflection(0.0f), kTransmission(0.0f), refractiveIndex(1.0f),
      kTexture(0.0f), comment("") {}

ModelMaterial::ModelMaterial(const std::string& toStringStr) {
    std::istringstream scanner(toStringStr);
    try {
        ParserUtils::parseTokenWithoutParameter(scanner, "Material");
        kColor = ParserUtils::parseTokenFloat(scanner, "kColor");
        
        // Extract the base solid color RGB values
        float cR = ParserUtils::parseTokenFloat(scanner, "color_R");
        float cG = ParserUtils::parseTokenFloat(scanner, "color_G");
        float cB = ParserUtils::parseTokenFloat(scanner, "color_B");
        color = glm::vec3(cR, cG, cB);
        
        kDirect = ParserUtils::parseTokenFloat(scanner, "kDirect");
        
        // Extract the ambient background illumination RGB values
        float kaR = ParserUtils::parseTokenFloat(scanner, "ka_R");
        float kaG = ParserUtils::parseTokenFloat(scanner, "ka_G");
        float kaB = ParserUtils::parseTokenFloat(scanner, "ka_B");
        ka = glm::vec3(kaR, kaG, kaB);
        
        // Extract the direct diffuse illumination RGB values
        float kdR = ParserUtils::parseTokenFloat(scanner, "kd_R");
        float kdG = ParserUtils::parseTokenFloat(scanner, "kd_G");
        float kdB = ParserUtils::parseTokenFloat(scanner, "kd_B");
        kd = glm::vec3(kdR, kdG, kdB);
        
        // Extract the specular highlight RGB values
        float ksR = ParserUtils::parseTokenFloat(scanner, "ks_R");
        float ksG = ParserUtils::parseTokenFloat(scanner, "ks_G");
        float ksB = ParserUtils::parseTokenFloat(scanner, "ks_B");
        ks = glm::vec3(ksR, ksG, ksB);
        
        // Extract interaction multipliers and material density
        shininess = ParserUtils::parseTokenFloat(scanner, "shininess");
        kReflection = ParserUtils::parseTokenFloat(scanner, "kReflection");
        kTransmission = ParserUtils::parseTokenFloat(scanner, "kTransmission");
        refractiveIndex = ParserUtils::parseTokenFloat(scanner, "refractiveIndex");
        kTexture = ParserUtils::parseTokenFloat(scanner, "kTexture");

        // Consume the rest of the line as an optional description
        comment = ParserUtils::parseTokenRestOfString(scanner, "comment");

    } catch (const std::exception& e) {
        std::string errorMessage = std::string(e.what()) + 
                                   "\nAt Material constructor from toStringStr.\n Fail to load parse string:\n" + 
                                   toStringStr;
        std::cerr << errorMessage << std::endl;
        throw std::runtime_error(errorMessage);
    }
}
