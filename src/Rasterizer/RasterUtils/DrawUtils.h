#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <cmath>
#include <utility>

namespace DrawUtils {

    /**
     * @brief Defines the expected function signature for plotting a single RGB color to a specific screen coordinate.
     * 
     * @param x The horizontal screen pixel coordinate.
     * @param y The vertical screen pixel coordinate.
     * @param color The final computed RGB color for the pixel.
     */
    using PlotPixelCallback = std::function<void(int x, int y, const glm::vec3& color)>;

    /**
     * @brief Draws a solid, continuous line, using Bresenham's algorithm, between two 2D screen coordinates by calculating 
     *        exactly which pixels to color along the path, guaranteeing no gaps in the line.
     * 
     * @param plotPixel The function executed to draw the calculated colors to the screen buffer.
     * @param p1 The starting 2D screen coordinate.
     * @param p2 The ending 2D screen coordinate.
     * @param color The RGB color to paint the line.
     */
    inline void drawLineBresenham(const PlotPixelCallback& plotPixel, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color) {
        int x1 = static_cast<int>(std::round(p1.x));
        int y1 = static_cast<int>(std::round(p1.y));
        float z1 = p1.z;

        int x2 = static_cast<int>(std::round(p2.x));
        int y2 = static_cast<int>(std::round(p2.y));
        float z2 = p2.z;

        int dx = x2 - x1;
        int dy = y2 - y1;

        // Swap the start and end points if the line is drawing backwards
        if (dy < -dx) {
            std::swap(x1, x2);
            std::swap(y1, y2);
            dx = x2 - x1;
            dy = y2 - y1;
        }

        // Determine if the line is more horizontal or vertical to iterate along the correct axis
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
                // Adjust the vertical coordinate when the mathematical line crosses into the next pixel row
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
                // Adjust the horizontal coordinate when the mathematical line crosses into the next pixel column
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