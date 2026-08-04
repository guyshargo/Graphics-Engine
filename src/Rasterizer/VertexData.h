#pragma once
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <sstream>

/**
 * @brief Represents the geometric and visual data for a single vertex, 
 *        used during the rasterization pipeline.
 */
struct VertexData {

    // Loaded from file
    glm::vec3 pointObjectCoordinates; // Position of the vertex in object coordinates
    glm::vec3 normalObjectCoordinates; // Normal vector at the vertex in object coordinates
    glm::vec2 textureCoordinates; // Texture coordinates for the vertex
    glm::vec3 color;

    // Calculated in vertex processing and used by face processing
    glm::vec3 pointEyeCoordinates; // Position of the vertex in eye coordinates
    glm::vec3 pointWindowCoordinates;   // Position of the vertex in window coordinates
    glm::vec3 normalEyeCoordinates; // Normal vector at the vertex in eye coordinates
    float lightingIntensity0to1;

    // Constructor
    VertexData(const glm::vec3& pointObj, const glm::vec3& normalObj, const glm::vec2& texCoord, const glm::vec3& col)
        : pointObjectCoordinates(pointObj), 
          normalObjectCoordinates(normalObj), 
          textureCoordinates(texCoord), 
          color(col),
          pointEyeCoordinates(0.0f),
          pointWindowCoordinates(0.0f),
          normalEyeCoordinates(0.0f),
          lightingIntensity0to1(0.0f) {}

    /**
     * @brief Returns a string representation of the vertex for debugging.
     * @return Formatted string containing the vertex coordinates.
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << "Vertex [p=(" << pointObjectCoordinates.x << ", "
            << pointObjectCoordinates.y << ", "
            << pointObjectCoordinates.z << ")]";
        return oss.str();
    }
};