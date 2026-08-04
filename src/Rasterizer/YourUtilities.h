#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace YourUtilities {

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

}