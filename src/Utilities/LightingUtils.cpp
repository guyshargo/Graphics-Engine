#include "LightingUtils.h"

namespace LightingUtils {
    glm::vec3 lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPosition, 
                               const glm::vec3& kd, const glm::vec3& ks, const glm::vec3& ka, float shininess) {
        glm::vec3 returnedColor(0.0f);

        glm::vec3 normal = glm::normalize(pointNormal);
        glm::vec3 lightDir = glm::normalize(lightPosition - point);

        // cos of angle between light direction and normal
        float NdotL = glm::dot(normal, lightDir);

        // Diffusive Lighting: max between 0 (back of the surface so no light) and NdotL
        float diffusiveLight = glm::max(NdotL, 0.0f);
        returnedColor += diffusiveLight * kd;

        // Specular Lighting
        // if light is in front of object calculate specular light
        if (NdotL >= 0) {
            // Calculate reflection vector R, where the light ray bounces to
            glm::vec3 specularR = glm::normalize(2.0f * NdotL * normal - lightDir);

            // Direction vector of eye - from intersection point to camera
            glm::vec3 specularV = glm::normalize(-point);

            // if returned ray and eye direction are opposite to each other take max
            float RdotV = glm::max(glm::dot(specularR, specularV), 0.0f);

            // Adding shininess factor - how concentrated is the shine
            float specularLight = glm::pow(RdotV, shininess);
            returnedColor += specularLight * ks;
        }

        // Ambient Lighting: constant light that is always present
        returnedColor += ka;

        return returnedColor;
    }

    float lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, 
                           float Kd, float Ks, float Ka, float shininess) {
        glm::vec3 color = lightingEquation(point, pointNormal, lightPos, glm::vec3(Kd), glm::vec3(Ks), glm::vec3(Ka), shininess);
        return color.x;
    }
}