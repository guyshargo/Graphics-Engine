#pragma once
#include <string>
#include <glm/glm.hpp>

/**
 * @brief Defines a light source in the 3D scene, specifying its position, brightness, 
 *        and physical size to calculate illumination and cast soft shadows.
 */
class ModelLight {
public:
    glm::vec3 location;
    float intensity;
    float radius;
    std::string comment;

    // Constructors
    ModelLight();

    /**
     * @brief Creates a light source with specific predefined attributes.
     * 
     * @param location The 3D coordinates where the light is placed.
     * @param intensity The overall brightness of the light.
     * @param radius The physical width of the light, used to calculate blurry shadow edges.
     * @param comment An optional text note describing the light.
     */
    ModelLight(glm::vec3 location, float intensity, float radius, const std::string& comment);
    
    /**
     * @brief Extracts light properties directly from a formatted line of text in the scene file.
     * 
     * @param toStringStr The raw text line containing the light's data.
     */
    explicit ModelLight(const std::string& toStringStr);

};