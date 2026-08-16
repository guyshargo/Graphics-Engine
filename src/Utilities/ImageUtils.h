#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

namespace ImageUtils {
    void WriteColorToBuffer(std::vector<uint32_t>& buffer, int x, int y, const glm::vec3& color, int imageWidth, int imageHeight);
}