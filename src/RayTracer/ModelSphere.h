#pragma once
#include <string>
#include <glm/glm.hpp>

class ModelSphere {
public:
    glm::vec3 center;
    float radius;
    int materialIndex;
    int textureIndex;

    // Constructors
    ModelSphere();
    ModelSphere(glm::vec3 center, float radius, int materialIndex, int textureIndex);
    
    // Parse from string constructor
    explicit ModelSphere(const std::string& toStringStr);

    // Equivalent to Java's toString()
    std::string toString() const;
};