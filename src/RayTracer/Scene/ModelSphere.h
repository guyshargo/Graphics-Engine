#pragma once
#include <string>
#include <glm/glm.hpp>

/**
 * @brief Defines a solid round object in the 3D scene, storing its exact location, size, 
 *        and which visual material and image texture to apply to its surface.
 */
class ModelSphere {
public:
    glm::vec3 center;
    float radius;
    int materialIndex;
    int textureIndex;

    // Constructors
    ModelSphere();

    /**
     * @brief Creates a sphere object with specific predefined attributes.
     * 
     * @param center The 3D coordinates of the sphere's exact center.
     * @param radius The distance from the center to the outer edge of the sphere.
     * @param materialIndex The ID linking this sphere to a specific ModelMaterial.
     * @param textureIndex The ID linking this sphere to a specific SphereTexture.
     */
    ModelSphere(glm::vec3 center, float radius, int materialIndex, int textureIndex);
    
    /**
     * @brief Extracts sphere properties directly from a formatted line of text in the scene file.
     * 
     * @param toStringStr The raw text line containing the sphere's data.
     */
    explicit ModelSphere(const std::string& toStringStr);

};