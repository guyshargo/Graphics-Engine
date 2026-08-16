#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <cmath>
#include <utility>

namespace DrawUtils {

    using PlotPixelCallback = std::function<void(int x, int y, const glm::vec3& color)>;

    inline void drawLineBresenham(const PlotPixelCallback& plotPixel, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color) {
        int x1 = static_cast<int>(std::round(p1.x));
        int y1 = static_cast<int>(std::round(p1.y));
        float z1 = p1.z;

        int x2 = static_cast<int>(std::round(p2.x));
        int y2 = static_cast<int>(std::round(p2.y));
        float z2 = p2.z;

        int dx = x2 - x1;
        int dy = y2 - y1;

        // First check if switching points is needed
        if (dy < -dx) {
            std::swap(x1, x2);
            std::swap(y1, y2);
            dx = x2 - x1;
            dy = y2 - y1;
        }

        if (std::abs(dy) <= std::abs(dx)) {
            int yInceremnt = 1;
            if (dy < 0) {
                yInceremnt = -1;
                dy = -dy;
            }
            int y = y1;
            int diff = 2 * dy - dx;

            for (int stepX = x1; stepX <= x2; stepX++) {
                plotPixel(stepX, y, color);
                if (diff < 0) {
                    diff += 2 * dy;
                } else {
                    y += yInceremnt;
                    diff += 2 * dy - 2 * dx;
                }
            }
        } else {
            int xInceremnt = 1;
            if (dx < 0) {
                xInceremnt = -1;
                dx = -dx;
            }
            int x = x1;
            int diff = 2 * dx - dy;

            for (int stepY = y1; stepY <= y2; stepY++) {
                plotPixel(x, stepY, color);
                if (diff < 0) {
                    diff += 2 * dx;
                } else {
                    x += xInceremnt;
                    diff += 2 * dx - 2 * dy;
                }
            }
        }   
    }
}