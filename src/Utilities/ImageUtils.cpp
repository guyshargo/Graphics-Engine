#include "ImageUtils.h"
#include <algorithm>

namespace ImageUtils {
    void WriteColorToBuffer(std::vector<uint32_t>& buffer, int x, int y, const glm::vec3& color, int imageWidth, int imageHeight) {

        // Drop pixels that fall outside the screen dimensions
        if (x < 0 || x >= imageWidth || y < 0 || y >= imageHeight) 
            return;
        
        // Clamp color values to prevent overflow, then scale to 0-255 RGB range
        uint8_t red = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
        uint8_t green = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
        uint8_t blue = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
        
        // Invert the Y coordinate so the image is written in the same top-left-origin layout SDL expects
        int flippedY = imageHeight - 1 - y;

        // Pack components into an ARGB 32-bit integer and insert into the 1D buffer
        buffer[flippedY * imageWidth + x] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
    }
}