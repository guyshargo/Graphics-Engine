#pragma once

#include "./Data/PipelineData.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace TransformUtils {

    inline static glm::mat4 createViewportMatrix(float x, float y, float width, float height) {
        glm::mat4 viewportMatrix(1.0f);

        // Scale normalized device coordinates to window coordinates
        viewportMatrix[0][0] = width / 2.0f;  // Scale X
        viewportMatrix[1][1] = height / 2.0f; // Scale Y
        viewportMatrix[2][2] = 0.5f;          // Scale Z

        // Translate the scaled coordinates to fit the viewport
        viewportMatrix[3][0] = x + width / 2.0f;  // Translate X
        viewportMatrix[3][1] = y + height / 2.0f; // Translate Y
        viewportMatrix[3][2] = 0.5f;              // Translate Z

        return viewportMatrix;
    }

    inline static std::vector<ClippingPlane> getViewPlanes(float fov, float aspectRatio, float nearPlane, float farPlane) {
        std::vector<ClippingPlane> planes;

        // Near & Far planes
        planes.emplace_back(glm::vec3(0.0f, 0.0f, -nearPlane), glm::vec3(0.0f, 0.0f, -1.0f));
        planes.emplace_back(glm::vec3(0.0f, 0.0f, -farPlane),  glm::vec3(0.0f, 0.0f, 1.0f));

        return planes;
    }
}