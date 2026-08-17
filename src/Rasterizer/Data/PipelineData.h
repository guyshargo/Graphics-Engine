#pragma once
#include <glm/glm.hpp>
#include <array>
#include <string>
#include <sstream>
#include <iostream>

/**
 * @brief Represents the geometric and visual data for a single vertex, 
 *        used and transformed throughout the rasterization pipeline stages.
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

    // Constructors
    VertexData() = default;
    
    /**
     * @brief Packages the raw 3D data extracted from a file into a format the rendering pipeline can use.
     * 
     * @param pointObj The 3D coordinates of the point.
     * @param normalObj The direction the surface faces at this specific point.
     * @param texCoord The 2D coordinates for image mapping.
     * @param col The base RGB color of the point.
     */
    VertexData(const glm::vec3& pointObj, const glm::vec3& normalObj, const glm::vec2& texCoord, const glm::vec3& col)
        : pointObjectCoordinates(pointObj), 
          normalObjectCoordinates(normalObj), 
          textureCoordinates(texCoord), 
          color(col),
          pointEyeCoordinates(0.0f),
          pointWindowCoordinates(0.0f),
          normalEyeCoordinates(0.0f),
          lightingIntensity0to1(0.0f) {}
};

/**
 * @brief Links three vertices together to form a solid flat surface that the engine can draw.
 */
struct TriangleFace {
    std::array<int, 3> indices;
    glm::vec3 color;
    glm::vec3 normal;

    /**
     * @brief Constructs a single geometric triangle linking three existing 3D points.
     * 
     * @param indicesList The array of three integer IDs linking to the master vertex array.
     * @param col The RGB color of the entire flat surface.
     * @param norm The mathematical direction the flat surface is facing.
     */
    TriangleFace(const std::array<int, 3>& indicesList, const glm::vec3& col, const glm::vec3& norm = glm::vec3(0.0f)) 
                : indices(indicesList), color(col), normal(norm) {}
};

/**
 * @brief The final package of calculated data for a single screen pixel, containing the 
 *        exact lighting direction and texture coordinate needed to physically color it.
 */
struct FragmentData {
    glm::vec3 pixelColor; // Color of the fragment
    glm::vec3 pointEyeCoordinates; // Position of the fragment in eye coordinates
    glm::vec3 normalEyeCoordinates; // Normal vector at the fragment in eye coordinates
    glm::vec2 textureCoordinates; // Texture coordinates for the fragment
    float pixelIntensity0to1; // Pixel intensity for the fragment
    float levelOfDetail; // Level of detail for Trilinear Filtering

    // Constructor
    FragmentData() = default;
};

/**
 * @brief A flat mathematical boundary used by the camera to slice off and discard 3D geometry 
 *        that falls entirely outside the visible screen area.
 */
struct ClippingPlane {
    glm::vec3 pointOnPlane;
    glm::vec3 normal;

    /**
     * @brief Defines the exact location and orientation of a cutting boundary in 3D space.
     * 
     * @param point A specific 3D coordinate the mathematical boundary passes through.
     * @param norm The direction the boundary faces, determining which side is visible and which is hidden.
     */
    ClippingPlane(const glm::vec3& point, const glm::vec3 norm) : pointOnPlane(point), normal(norm) {};
};