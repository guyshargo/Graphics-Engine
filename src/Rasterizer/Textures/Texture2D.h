#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct MipLevel {
    int width;
    int height;
    std::vector<glm::vec3> texData;
};

class Texture2D {
public:
    explicit Texture2D(const std::string& filepath);
    
    bool isValid() const { return loaded; }

    int getWidth() const { return mipmaps.empty() ? 0 : mipmaps[0].width; }
    int getHeight() const { return mipmaps.empty() ? 0 : mipmaps[0].height; }
    
    // Evaluates bilinear filtering across the two closest mipmap levels based on LOD
    glm::vec3 sample(float levelOfDetail, const glm::vec2& texCoordinates) const;

private:
    std::vector<MipLevel> mipmaps;
    bool loaded = false;

    glm::vec3 sampleBilinear(int mipmapLevel, const glm::vec2& texCoordinates) const;
};