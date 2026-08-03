#pragma once
#include <string>
#include <stdexcept>
#include <glm/glm.hpp>

/**
 * The SphereTexture class represents a texture mapped onto a sphere and
 * provides methods to sample pixel colors based on direction vectors. 
 */
class SphereTexture {
public:
    /**
     * Constructs a SphereTexture object by loading the image from the
     * specified file path.
     * Throws std::runtime_error if the image fails to load.
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
     * Samples the color from the texture based on a given direction vector.
     */
    glm::vec3 sampleDirectionFromMiddle(const glm::vec3& direction) const;

private:
    int width;
    int height;
    int channels;
    unsigned char* imageData; // Using unsigned char replaces Java's byte array
    std::string filename;

    glm::vec3 getImagePixel(int x, int y) const;
    void loadImage(const std::string& filepath);
};