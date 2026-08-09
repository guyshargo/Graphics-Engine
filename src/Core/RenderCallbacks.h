#pragma once

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include "RayTracerWorld.h"
#include "Utilities.h"
#include "DefaultParams.h"

// --------------------------------------------------------
// Rasterizer Callbacks
// --------------------------------------------------------
struct ImageClearer {
    std::vector<uint32_t>& pixelBuffer;

    ImageClearer(std::vector<uint32_t>& buffer) : pixelBuffer(buffer) {}

    void operator()() const {
        std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0xFF323232);
    }
};

struct PixelSetter {
    std::vector<uint32_t>& pixelBuffer;

    PixelSetter(std::vector<uint32_t>& buffer) : pixelBuffer(buffer) {}

    void operator()(int x, int y, const glm::vec3& color) const {
        Utilities::WriteColorToBuffer(pixelBuffer, x, y, color, DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
    }
};

// --------------------------------------------------------
// Ray Tracer Multi-Threading Task
// --------------------------------------------------------
struct PixelRenderer {
    std::vector<uint32_t>& pixelBuffer;
    RayTracerWorld& rayTracerWorld;

    PixelRenderer(std::vector<uint32_t>& buffer, RayTracerWorld& rtWorld) 
        : pixelBuffer(buffer), rayTracerWorld(rtWorld) {}

    void operator()(int pixelIndex) const {
        int x = pixelIndex % DefaultParams::IMAGE_WIDTH;
        int y = pixelIndex / DefaultParams::IMAGE_WIDTH;

        glm::vec3 color = rayTracerWorld.renderPixel(x, y);
        
        Utilities::WriteColorToBuffer(pixelBuffer, x, y, color, DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
    }
};