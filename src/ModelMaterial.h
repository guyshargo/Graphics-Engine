#pragma once
#include <glm/glm.hpp>
#include <string>

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

    ModelMaterial()
        : kColor(0.0f), color(0.0f), kDirect(0.0f), ka(0.0f), kd(0.0f), ks(0.0f),
          shininess(1.0f), kReflection(0.0f), kTransmission(0.0f), refractiveIndex(1.0f),
          kTexture(0.0f), comment("") {}
};