#pragma once

#include "./Data/PipelineData.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace TransformUtils {

    /**
     * @brief Builds the matrix that stretches and shifts the flattened 2D coordinates 
     *        so they perfectly fit the final pixel dimensions of the rendering screen.
     * 
     * @param x The horizontal starting coordinate of the rendering area (usually 0).
     * @param y The vertical starting coordinate of the rendering area (usually 0).
     * @param width The total pixel width of the rendering screen.
     * @param height The total pixel height of the rendering screen.
     * @return The calculated 4x4 viewport transformation matrix.
     */
    inline static glm::mat4 createViewportMatrix(float x, float y, float width, float height) {
        glm::mat4 viewportMatrix(1.0f);

        // Scale normalized device coordinates (-1.0 to 1.0) into absolute window dimensions
        viewportMatrix[0][0] = width / 2.0f;  // Scale X
        viewportMatrix[1][1] = height / 2.0f; // Scale Y
        viewportMatrix[2][2] = 0.5f;          // Scale Z

        // Shift the scaled coordinates to position them correctly inside the window boundaries
        viewportMatrix[3][0] = x + width / 2.0f;  // Translate X
        viewportMatrix[3][1] = y + height / 2.0f; // Translate Y
        viewportMatrix[3][2] = 0.5f;              // Translate Z

        return viewportMatrix;
    }

    /**
     * @brief Defines the invisible 3D boundaries in front of and behind the camera, allowing 
     *        the engine to slice off and discard geometry that is too close or too far away to see.
     * 
     * @param fov The horizontal field of view angle in degrees.
     * @param aspectRatio The ratio of the screen's width to its height.
     * @param nearPlane The minimum 3D distance an object must be from the camera to be visible.
     * @param farPlane The maximum 3D distance an object can be from the camera before it disappears.
     * @return A list containing the mathematical clipping planes used by the geometry slicer.
     */
    inline static std::vector<ClippingPlane> getViewPlanes(float fov, float aspectRatio, float nearPlane, float farPlane) {
        std::vector<ClippingPlane> planes;

        // Define the flat boundaries perpendicular to the camera's viewing direction (Near & Far planes)
        planes.emplace_back(glm::vec3(0.0f, 0.0f, -nearPlane), glm::vec3(0.0f, 0.0f, -1.0f));
        planes.emplace_back(glm::vec3(0.0f, 0.0f, -farPlane),  glm::vec3(0.0f, 0.0f, 1.0f));

        return planes;
    }
}