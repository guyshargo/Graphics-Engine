#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace ImageUtils {

    /**
     * @brief Takes the final computed color for a single point in the 3D world and plots it 
     *        directly into the 2D pixel array that is presented on the user's monitor, handling 
     *        the necessary conversions into standard screen color formats.
     * 
     * @param buffer The 1D integer vector representing the ARGB image payload.
     * @param x The target pixel's X coordinate.
     * @param y The target pixel's Y coordinate.
     * @param color The normalized (0.0 to 1.0) RGB color vector.
     * @param imageWidth The total width of the image.
     * @param imageHeight The total height of the image.
     */
    void WriteColorToBuffer(std::vector<uint32_t>& buffer, int x, int y, const glm::vec3& color, int imageWidth, int imageHeight);
}