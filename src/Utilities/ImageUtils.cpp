#include "ImageUtils.h"
#include <algorithm>

namespace ImageUtils {
    void WriteColorToBuffer(std::vector<uint32_t>& buffer, int x, int y, const glm::vec3& color, int imageWidth, int imageHeight) {
        if (x < 0 || x >= imageWidth || y < 0 || y >= imageHeight) 
            return;
        
        uint8_t red = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
        uint8_t green = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
        uint8_t blue = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
        
        int flippedY = imageHeight - 1 - y;
        buffer[flippedY * imageWidth + x] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
    }
}