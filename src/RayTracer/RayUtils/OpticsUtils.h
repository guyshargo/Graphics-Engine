#pragma once
#include <glm/glm.hpp>
#include <cmath>

namespace OpticsUtils {

    /**
     * @brief Calculates the direction of a transmission ray as it passes through
     *        a surface defined by a normal vector, using Snell's Law to account
     *        for refraction.
     * 
     * @param incidentRay The direction the light ray is currently traveling before hitting the surface.
     * @param normal The normal vector defining the surface at the point of intersection.
     * @param refractiveIndexIntersectedSphere The refractive index of the material the ray is interacting with.
     * @param rayFromOutside True if the ray is entering the material, false if it is exiting.
     * @return The calculated transmission ray direction, or a zero vector if total internal reflection occurs.
     */
    inline glm::vec3 calcTransmissionRay(const glm::vec3& incidentRay, 
                                         const glm::vec3& normal,
                                         float refractiveIndexIntersectedSphere, 
                                         bool rayFromOutside) {
                                             
        // Constant for air's refractive index
        const float refractiveIndexAir = 1.000293f; 
        float n1, n2;

        // Normalize inputs
        glm::vec3 incidentRay_ = glm::normalize(incidentRay);
        glm::vec3 normal_ = glm::normalize(normal);

        // Determine indices of refraction based on the ray direction
        if (rayFromOutside) {
            n1 = refractiveIndexAir;
            n2 = refractiveIndexIntersectedSphere;
        } else {
            n1 = refractiveIndexIntersectedSphere;
            n2 = refractiveIndexAir;
        }

        // Calculate dot product of incident ray and normal
        float cosTheta1 = glm::dot(incidentRay_, normal_);

        // Invert the normal if the ray is inside the object
        if (cosTheta1 < 0.0f) {
            cosTheta1 = -cosTheta1;
        } else {
            normal_ = -normal_;
        }

        // Calculate the ratio of refractive indices and squared terms for Snell's law
        float ratio = n1 / n2;
        float sinTheta2Sq = ratio * ratio * (1.0f - cosTheta1 * cosTheta1);

        // Check for total internal reflection (if sin^2(theta2) > 1.0)
        if (sinTheta2Sq > 1.0f) {
            return glm::vec3(0.0f); // Total internal reflection
        }

        // Calculate cosTheta2 using trigonometric identity
        float cosTheta2 = std::sqrt(1.0f - sinTheta2Sq);

        // Compute the transmission direction using Snell's law
        glm::vec3 transmittedDir = (incidentRay_ * ratio) + (normal_ * (ratio * cosTheta1 - cosTheta2));

        return glm::normalize(transmittedDir); // Return the normalized direction
    }

}