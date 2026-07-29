#pragma once
#include <glm/glm.hpp>
#include <string>

class ModelLight {
public:
    glm::vec3 location;
    float intensity;
    std::string comment;

    ModelLight(glm::vec3 location, float intensity, std::string comment)
        : location(location), intensity(intensity), comment(comment) {}

    ModelLight()
        : location(0.0f), intensity(1.0f), comment("") {}
};