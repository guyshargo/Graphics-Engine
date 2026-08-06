#pragma once

#include <string>
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include <SDL.h>

#include "VertexData.h"
#include "TriangleFace.h"

class OBJLoader {
public:
    OBJLoader(bool useSphericalMapping = true);
    
    void loadOBJ(const std::string& filePath);

    glm::vec3 getBoundingBoxDimensions() const;
    glm::vec3 getBoundingBoxCenter() const;

    const std::vector<VertexData>& getVertices() const { return vertices; }
    const std::vector<TriangleFace>& getFaces() const { return faces; }
    const std::string& getTextureFilePath() const { return textureFilePath; }

private:
    void calculateNormalsSmoothShading();
    void calculateSphericalMapping();
    void calculatePlanarMapping();

    bool useSphericalMapping;
    std::vector<VertexData> vertices;
    std::vector<TriangleFace> faces;
    std::string textureFilePath;

    float minPositionX, maxPositionX;
    float minPositionY, maxPositionY;
    float minPositionZ, maxPositionZ;
};