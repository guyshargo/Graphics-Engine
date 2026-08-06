#pragma once
#include <string>
#include <glm/glm.hpp>

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
    
    // Parse from string constructor
    explicit ModelMaterial(const std::string& toStringStr);

    // Equivalent to Java's toString()
    std::string toString() const;
};