#pragma once
#include <string>
#include <stdexcept>
#include <glm/glm.hpp>

/**
 * @brief The SphereTexture class represents a texture mapped onto a sphere and
 *        provides methods to sample pixel colors based on direction vectors. 
 */
class SphereTexture {
public:
    /**
     * @brief Loads an image from the hard drive to be used as a wraparound texture.
     * 
     * @param filepath The system path to the image file.
     */
    SphereTexture(const std::string& filepath);

    /**
     * Destructor to safely clean up the dynamically allocated image memory.
     */
    ~SphereTexture();

    // Delete copy constructor and assignment operator to prevent accidental double-frees of image memory
    SphereTexture(const SphereTexture&) = delete;
    SphereTexture& operator=(const SphereTexture&) = delete;

    SphereTexture(SphereTexture&& other) noexcept;
    SphereTexture& operator=(SphereTexture&& other) noexcept;

    /**
     * @brief Finds the exact pixel color on the image based on which direction the light ray is pointing.
     * 
     * @param direction The 3D trajectory of the light ray looking out into the scene.
     * @return The specific RGB color of the image at that mapped coordinate.
     */
    glm::vec3 sampleDirectionFromMiddle(const glm::vec3& direction) const;

private:
    int width;
    int height;
    int channels;
    unsigned char* imageData;
    std::string filename;

    glm::vec3 getImagePixel(int x, int y) const;
    void loadImage(const std::string& filepath);
};