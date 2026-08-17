#pragma once
#include <string>
#include <glm/glm.hpp>

/**
 * @brief Defines the visual properties of a surface, controlling its base color, shininess, 
 *        reflectiveness, and transparency to determine how it reacts to light.
 */
class ModelMaterial {
public:
    float kColor;
    glm::vec3 color;

    float kDirect;
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    float shininess;

    float kReflection;
    float kTransmission;
    float refractiveIndex;
    float kTexture;

    std::string comment;

    // Constructors
    ModelMaterial();
    
    /**
     * @brief Extracts material properties directly from a formatted line of text in the scene file.
     * 
     * @param toStringStr The raw text line containing the material's data.
     */
    explicit ModelMaterial(const std::string& toStringStr);

};