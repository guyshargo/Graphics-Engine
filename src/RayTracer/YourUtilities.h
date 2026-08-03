#pragma once
#include <glm/glm.hpp>
#include <cmath>

namespace YourUtilities {

    /**
     * Calculates the direction of a transmission ray as it passes through
     * a surface defined by a normal vector, using Snell's Law to account
     * for refraction.
     */
    inline glm::vec3 calcTransmissionRay(const glm::vec3& incidentRay, 
                                         const glm::vec3& normal,
                                         float refractiveIndexIntersectedSphere, 
                                         bool rayFromOutside) {
                                             
        // Constant for air's refractive index[cite: 5]
        const float refractiveIndexAir = 1.000293f; 
        float n1, n2;

        // Normalize inputs[cite: 5]
        glm::vec3 incidentRay_ = glm::normalize(incidentRay);
        glm::vec3 normal_ = glm::normalize(normal);

        // Determine indices of refraction based on the ray direction[cite: 5]
        if (rayFromOutside) {
            n1 = refractiveIndexAir;
            n2 = refractiveIndexIntersectedSphere;
        } else {
            n1 = refractiveIndexIntersectedSphere;
            n2 = refractiveIndexAir;
        }

        // Calculate dot product of incident ray and normal[cite: 5]
        float cosTheta1 = glm::dot(incidentRay_, normal_);

        // Invert the normal if the ray is inside the object[cite: 5]
        if (cosTheta1 < 0.0f) {
            cosTheta1 = -cosTheta1;
        } else {
            normal_ = -normal_;
        }

        // Calculate the ratio of refractive indices and squared terms for Snell's law[cite: 5]
        float ratio = n1 / n2;
        float sinTheta2Sq = ratio * ratio * (1.0f - cosTheta1 * cosTheta1);

        // Check for total internal reflection (if sin^2(theta2) > 1.0)[cite: 5]
        if (sinTheta2Sq > 1.0f) {
            return glm::vec3(0.0f); // Total internal reflection[cite: 5]
        }

        // Calculate cosTheta2 using trigonometric identity[cite: 5]
        float cosTheta2 = std::sqrt(1.0f - sinTheta2Sq);

        // Compute the transmission direction using Snell's law[cite: 5]
        glm::vec3 transmittedDir = (incidentRay_ * ratio) + (normal_ * (ratio * cosTheta1 - cosTheta2));

        return glm::normalize(transmittedDir); // Return the normalized direction[cite: 5]
    }

}