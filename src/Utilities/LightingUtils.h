#pragma once
#include <glm/glm.hpp>

namespace LightingUtils {
    glm::vec3 lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, 
                               const glm::vec3& Kd, const glm::vec3& Ks, const glm::vec3& Ka, float shininess);

    // Overload for uniform float parameters
    float lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, 
                           float Kd, float Ks, float Ka, float shininess);
}