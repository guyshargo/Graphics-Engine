#pragma once
#include <glm/glm.hpp>

namespace LightingUtils {

    /**
     * @brief Calculates the surface light intensity using ambient, diffuse, and specular components.
     * 
     * @param point The 3D world/eye coordinates of the surface point being lit.
     * @param pointNormal The normalized surface normal at the point.
     * @param lightPos The 3D coordinates of the light source.
     * @param Kd The diffuse reflection coefficient (RGB color).
     * @param Ks The specular reflection coefficient (RGB color).
     * @param Ka The ambient reflection coefficient (RGB color).
     * @param shininess The specular exponent determining the size of the reflection highlight.
     * @return A glm::vec3 representing the final computed RGB color.
     */
    glm::vec3 lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, 
                               const glm::vec3& Kd, const glm::vec3& Ks, const glm::vec3& Ka, float shininess);

    /**
     * @brief Calculates a monochromatic surface light intensity using ambient, diffuse, and specular components.
     * 
     * @param point The 3D world/eye coordinates of the surface point being lit.
     * @param pointNormal The normalized surface normal at the point.
     * @param lightPos The 3D coordinates of the light source.
     * @param Kd The diffuse reflection intensity (0.0 to 1.0).
     * @param Ks The specular reflection intensity (0.0 to 1.0).
     * @param Ka The ambient reflection intensity (0.0 to 1.0).
     * @param shininess The specular exponent determining the size of the reflection highlight.
     * @return A float representing the final computed grayscale lighting intensity.
     */
    float lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, 
                           float Kd, float Ks, float Ka, float shininess);
}