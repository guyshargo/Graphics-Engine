#pragma once
#include <glm/glm.hpp>

struct FragmentData {
    glm::vec3 pixelColor; // Color of the fragment
    glm::vec3 pointEyeCoordinates; // Position of the fragment in eye coordinates
    glm::vec3 normalEyeCoordinates; // Normal vector at the fragment in eye coordinates
    glm::vec2 textureCoordinates; // Texture coordinates for the fragment
    float lightingIntensity0to1; // Lighting intensity for the fragment

    // Constructor to initialize the fragment data
    FragmentData(const glm::vec3& posEye, const glm::vec3& normEye, const glm::vec2& tex, const glm::vec3& col, float intensity)
        : pixelColor(col), pointEyeCoordinates(posEye), normalEyeCoordinates(normEye), textureCoordinates(tex), lightingIntensity0to1(intensity) {}
};
