#pragma once
#include <glm/glm.hpp>

class ModelSphere {
public:
    glm::vec3 center;
    float radius;
    int materialIndex;
    int textureIndex;

    ModelSphere(glm::vec3 center, float radius, int materialIndex, int textureIndex)
        : center(center), radius(radius), materialIndex(materialIndex), textureIndex(textureIndex) {}

    ModelSphere()
        : center(0.0f, 0.0f, 0.0f), radius(0.0f), materialIndex(0), textureIndex(0) {}
};