#include "Texture2D.h"
#include <SDL.h>
#include <iostream>
#include <algorithm>
#include <cmath>

Texture2D::Texture2D(const std::string& filepath) {
    SDL_Surface* loadedTextureSurface = SDL_LoadBMP(filepath.c_str());
    if (!loadedTextureSurface) return;

    // Force the image decoder to convert the loaded image into a standard 32-bit ARGB pixel format
    SDL_Surface* convertedTextureSurface = SDL_ConvertSurfaceFormat(loadedTextureSurface, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(loadedTextureSurface);
    
    if (!convertedTextureSurface) return;

    MipLevel baseLevel;
    baseLevel.width = convertedTextureSurface->w;
    baseLevel.height = convertedTextureSurface->h;
    baseLevel.texData.resize(baseLevel.width * baseLevel.height);

    uint32_t* pixels = static_cast<uint32_t*>(convertedTextureSurface->pixels);
    int pitch = convertedTextureSurface->pitch / 4;

    // Loop through the image and divide the 0-255 color values into the 0.0-1.0 range used by the engine
    for (int y = 0; y < baseLevel.height; y++) {
        for (int x = 0; x < baseLevel.width; x++) {
            uint32_t argb = pixels[y * pitch + x];
            float r = static_cast<float>((argb >> 16) & 0xFF) / 255.0f;
            float g = static_cast<float>((argb >> 8) & 0xFF) / 255.0f;
            float b = static_cast<float>(argb & 0xFF) / 255.0f;
            
            // Flip the Y-axis to match standard graphics mapping coordinates
            int wrapY = baseLevel.height - 1 - y;
            baseLevel.texData[wrapY * baseLevel.width + x] = glm::vec3(r, g, b);
        }
    }
    SDL_FreeSurface(convertedTextureSurface);
    mipmaps.push_back(baseLevel);

    // Generate the mipmap chain by repeatedly dividing the image dimensions in half and averaging 4 pixels into 1
    while (mipmaps.back().width > 1 || mipmaps.back().height > 1) {
        const MipLevel& prev = mipmaps.back();
        MipLevel next;
        next.width = std::max(1, prev.width / 2);
        next.height = std::max(1, prev.height / 2);
        next.texData.resize(next.width * next.height);

        for (int y = 0; y < next.height; ++y) {
            for (int x = 0; x < next.width; ++x) {
                int px = x * 2;
                int py = y * 2;
                
                glm::vec3 c00 = prev.texData[py * prev.width + px];
                glm::vec3 c10 = prev.texData[py * prev.width + std::min(px + 1, prev.width - 1)];
                glm::vec3 c01 = prev.texData[std::min(py + 1, prev.height - 1) * prev.width + px];
                glm::vec3 c11 = prev.texData[std::min(py + 1, prev.height - 1) * prev.width + std::min(px + 1, prev.width - 1)];
                
                next.texData[y * next.width + x] = (c00 + c10 + c01 + c11) * 0.25f;
            }
        }
        mipmaps.push_back(next);
    }
    loaded = true;
}

glm::vec3 Texture2D::sampleBilinear(int mipmapLevel, const glm::vec2& texCoordinates) const {
    if (mipmapLevel >= mipmaps.size()) {
        mipmapLevel = static_cast<int>(mipmaps.size()) - 1;
    }
    const MipLevel& mip = mipmaps[mipmapLevel];
    
    glm::vec2 texImgCoords(texCoordinates.x * (mip.width - 1), texCoordinates.y * (mip.height - 1));

    int x0 = static_cast<int>(std::floor(texImgCoords.x));
    int y0 = static_cast<int>(std::floor(texImgCoords.y));
    int x1 = (x0 + 1) % mip.width;
    int y1 = (y0 + 1) % mip.height;

    float u_ratio = texImgCoords.x - x0;
    float v_ratio = texImgCoords.y - y0;

    glm::vec3 tex00 = mip.texData[y0 * mip.width + x0];
    glm::vec3 tex10 = mip.texData[y0 * mip.width + x1];
    glm::vec3 tex01 = mip.texData[y1 * mip.width + x0];
    glm::vec3 tex11 = mip.texData[y1 * mip.width + x1];

    glm::vec3 colorTop = glm::mix(tex00, tex10, u_ratio);
    glm::vec3 colorBottom = glm::mix(tex01, tex11, u_ratio);
    return glm::mix(colorTop, colorBottom, v_ratio);
}

glm::vec3 Texture2D::sample(float levelOfDetail, const glm::vec2& texCoordinates) const {
    if (!loaded || mipmaps.empty()) return glm::vec3(1.0f, 0.0f, 1.0f); // Magenta error fallback

    int level1 = static_cast<int>(std::floor(levelOfDetail));
    int level2 = std::min(level1 + 1, static_cast<int>(mipmaps.size() - 1));
    float fractionalLOD = levelOfDetail - level1;

    glm::vec3 color1 = sampleBilinear(level1, texCoordinates);
    glm::vec3 color2 = sampleBilinear(level2, texCoordinates);

    return glm::mix(color1, color2, fractionalLOD);
}