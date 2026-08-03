#include "SphereTexture.h"
#include <cmath>
#include <algorithm>
#include <glm/gtc/constants.hpp> // Access to glm::pi<float>()

// We define STB_IMAGE_IMPLEMENTATION here to tell stb_image to create the implementation code.
// This must only be done in ONE .cpp file in your entire project.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SphereTexture::SphereTexture(const std::string& filepath) 
    : width(-1), height(-1), channels(0), imageData(nullptr), filename(filepath) 
{
    loadImage(filepath);
}

SphereTexture::~SphereTexture() {
    // Safely free the memory allocated by stb_image when the object is destroyed
    if (imageData) {
        stbi_image_free(imageData);
    }
}

void SphereTexture::loadImage(const std::string& filepath) {
    // stbi_load automatically decodes the JPG/PNG. 
    // Passing '3' as the last argument forces it to load RGB channels, ignoring alpha if present.
    imageData = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
    
    if (!imageData) {
        throw std::runtime_error("Failed to load image: " + filepath);
    }
}

glm::vec3 SphereTexture::getImagePixel(int x, int y) const {
    int index = (y * width + x) * 3;
    
    // In C++, unsigned char ranges from 0 to 255 natively.
    // We do not need the bitwise AND (& 0xFF) workaround you used in Java.
    return glm::vec3(
        imageData[index] / 255.0f,       // Red
        imageData[index + 1] / 255.0f,   // Green
        imageData[index + 2] / 255.0f    // Blue
    );
}

glm::vec3 SphereTexture::sampleDirectionFromMiddle(const glm::vec3& direction) const {
    bool interpolate = true;

    float directionLength = glm::length(direction);
    glm::vec3 normalizedDirection = direction;
    
    // Normalize safely to prevent division by zero
    if (directionLength != 0.0f) {
        normalizedDirection = direction / directionLength; 
    }

    // Calculate texture coordinates from the direction vector
    float u = 0.5f + (std::atan2(normalizedDirection.z, normalizedDirection.x) / (2.0f * glm::pi<float>()));
    float v = 0.5f - (std::asin(normalizedDirection.y) / glm::pi<float>());

    if (interpolate) {
        // Get pixel coordinates
        int x0 = static_cast<int>(u * (width - 1));
        int y0 = static_cast<int>(v * (height - 1));
        int x1 = std::min(x0 + 1, width - 1);
        int y1 = std::min(y0 + 1, height - 1);

        // Weights for interpolation
        float wx0 = (x1 - u * (width - 1)) * (y1 - v * (height - 1));
        float wx1 = (u * (width - 1) - x0) * (y1 - v * (height - 1));
        float wx2 = (x1 - u * (width - 1)) * (v * (height - 1) - y0);
        float wx3 = (u * (width - 1) - x0) * (v * (height - 1) - y0);

        // Sample colors
        glm::vec3 c0 = getImagePixel(x0, y0);
        glm::vec3 c1 = getImagePixel(x1, y0);
        glm::vec3 c2 = getImagePixel(x0, y1);
        glm::vec3 c3 = getImagePixel(x1, y1);

        // GLM operator overloading makes this math incredibly clean compared to Java
        return c0 * wx0 + c1 * wx1 + c2 * wx2 + c3 * wx3;
    } else {
        // No interpolation, sample the nearest pixel
        int x = static_cast<int>(u * (width - 1));
        int y = static_cast<int>(v * (height - 1));
        return getImagePixel(x, y);
    }
}


// Move Constructor
SphereTexture::SphereTexture(SphereTexture&& other) noexcept 
    : width(other.width), 
      height(other.height), 
      channels(other.channels),
      imageData(other.imageData), 
      filename(std::move(other.filename)) 
{
    other.imageData = nullptr;
    other.width = -1;
    other.height = -1;
    other.channels = 0;
}

// Move Assignment Operator
SphereTexture& SphereTexture::operator=(SphereTexture&& other) noexcept {
    if (this != &other) {
        if (imageData) {
            stbi_image_free(imageData);
        }

        width = other.width;
        height = other.height;
        channels = other.channels;
        imageData = other.imageData;
        filename = std::move(other.filename);

        other.imageData = nullptr;
        other.width = -1;
        other.height = -1;
        other.channels = 0;
    }
    return *this;
}