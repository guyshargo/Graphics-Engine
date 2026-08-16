#pragma once
#include <string>
#include <glm/glm.hpp>

class ModelLight {
public:
    glm::vec3 location;
    float intensity;
    float radius;
    std::string comment;

    // Constructors
    ModelLight();
    ModelLight(glm::vec3 location, float intensity, float radius, const std::string& comment);
    
    // Parse from string constructor
    explicit ModelLight(const std::string& toStringStr);

    // Equivalent to Java's toString()
    std::string toString() const;
};