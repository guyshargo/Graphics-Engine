#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

/**
 * @brief Stores a single resolution tier of the image used in the mipmap chain.
 */
struct MipLevel {
    int width;
    int height;
    std::vector<glm::vec3> texData;
};

/**
 * @brief Loads a 2D image and automatically generates a chain of smaller, blurrier versions 
 *        (mipmaps) to prevent the texture from shimmering or looking pixelated when the 
 *        3D object moves far away from the camera.
 */
class Texture2D {
public:
    /**
     * @brief Loads the image file from the hard drive and builds the full mipmap resolution chain.
     * 
     * @param filepath The system path to the image file.
     */
    explicit Texture2D(const std::string& filepath);
    
    /**
     * @brief Checks if the image successfully loaded into memory.
     * 
     * @return True if the texture is ready to use, false if the file was missing or invalid.
     */
    bool isValid() const { return loaded; }

    /**
     * @brief Retrieves the width of the original, full-resolution image.
     * 
     * @return The width in pixels.
     */
    int getWidth() const { return mipmaps.empty() ? 0 : mipmaps[0].width; }

    /**
     * @brief Retrieves the height of the original, full-resolution image.
     * 
     * @return The height in pixels.
     */
    int getHeight() const { return mipmaps.empty() ? 0 : mipmaps[0].height; }
    
    /**
     * @brief Looks up the exact color of the image at a specific coordinate, blending between 
     *        different resolution levels depending on how far away or angled the surface is.
     * 
     * @param levelOfDetail The calculated float determining which two mipmap levels to blend between.
     * @param texCoordinates The 2D coordinates mapping the surface to the image.
     * @return The final filtered RGB color.
     */
    glm::vec3 sample(float levelOfDetail, const glm::vec2& texCoordinates) const;

private:
    std::vector<MipLevel> mipmaps;
    bool loaded = false;

    /**
     * @brief Calculates a smooth color by averaging the four nearest pixels at a specific mipmap resolution.
     * 
     * @param mipmapLevel The specific integer resolution tier to sample from.
     * @param texCoordinates The 2D coordinates mapping the surface to the image.
     * @return The averaged RGB color.
     */
    glm::vec3 sampleBilinear(int mipmapLevel, const glm::vec2& texCoordinates) const;
};