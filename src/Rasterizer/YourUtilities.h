#pragma once

#include "PlaneData.h"

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

    inline static std::vector<Plane> getViewPlanes(float fov, float aspectRatio, float nearPlane, float farPlane) {
        std::vector<Plane> planes;

        // Near & Far planes
        planes.emplace_back(glm::vec3(0.0f, 0.0f, -nearPlane), glm::vec3(0.0f, 0.0f, -1.0f));
        planes.emplace_back(glm::vec3(0.0f, 0.0f, -farPlane),  glm::vec3(0.0f, 0.0f, 1.0f));

        // Calculate the dimensions of the Near Plane
        float halfHeight = nearPlane * std::tan(glm::radians(fov) / 2.0f);
        float halfWidth = halfHeight * aspectRatio;

        // Define the exact coordinate vectors for the 4 corners of the Near Plane
        glm::vec3 ntl(-halfWidth,  halfHeight, -nearPlane); // Near Top Left
        glm::vec3 ntr( halfWidth,  halfHeight, -nearPlane); // Near Top Right
        glm::vec3 nbl(-halfWidth, -halfHeight, -nearPlane); // Near Bottom Left
        glm::vec3 nbr( halfWidth, -halfHeight, -nearPlane); // Near Bottom Right
        glm::vec3 origin(0.0f, 0.0f, 0.0f);

        // Top, Bottom, Left, Right planes
        planes.emplace_back(origin, glm::normalize(glm::cross(ntl, ntr))); // Top
        planes.emplace_back(origin, glm::normalize(glm::cross(nbr, nbl))); // Bottom
        planes.emplace_back(origin, glm::normalize(glm::cross(nbl, ntl))); // Left
        planes.emplace_back(origin, glm::normalize(glm::cross(ntr, nbr))); // Right

        return planes;
    }

}