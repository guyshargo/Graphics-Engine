#pragma once
#include <glm/glm.hpp>

struct FragmentData {
    glm::vec3 pixelColor; // Color of the fragment
    glm::vec3 pointEyeCoordinates; // Position of the fragment in eye coordinates
    glm::vec3 normalEyeCoordinates; // Normal vector at the fragment in eye coordinates
    glm::vec2 textureCoordinates; // Texture coordinates for the fragment
    float pixelIntensity0to1; // Pixel intensity for the fragment

    // Constructor
    FragmentData() = default;
};
