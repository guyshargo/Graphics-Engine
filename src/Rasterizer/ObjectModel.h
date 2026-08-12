#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VertexData.h"
#include "TriangleFace.h"
#include "ExerciseEnum.h"
#include "FragmentData.h"
#include "OBJLoader.h"

class RasterizerWorld;

class ObjectModel {
public:

    // Constructor
    ObjectModel(RasterizerWorld* rasterizerWorld, int imageWidth, int imageHeight);

    void initTransformations();
    void setModelM(const glm::mat4& modelM);
    void setLookatM(const glm::mat4& lookatM);
    void setProjectionM(const glm::mat4& projectionM);
    void setViewportM(const glm::mat4& viewportM);

    glm::vec3 getBoundingBoxDimensions() const;
    glm::vec3 getBoundingBoxCenter() const;

    bool load(const std::string& fileName);
    bool objectHasTexture() const;

    using PlotPixelCallback = std::function<void(int x, int y, const glm::vec3& color)>;
    using TextureGetPixel = std::function<glm::vec3(int x, int y)>;
    using TextureGetDimension = std::function<int()>;
    
    void setTextureCallbacks(TextureGetPixel getPixel, TextureGetDimension getWidth, TextureGetDimension getHeight);
    void render(const PlotPixelCallback& plotPixel);

    static RasterizationExerciseEnum exercise;

private:
    RasterizerWorld* rasterizerWorld;
    int imageWidth;
    int imageHeight;

    std::vector<VertexData> verticesData;
    std::vector<TriangleFace> faces;

    std::vector<glm::vec3> textureData;
    int textureWidth = 0;
    int textureHeight = 0;
    
    TextureGetPixel textureGetPixel;
    TextureGetDimension textureGetWidth;
    TextureGetDimension textureGetHeight;
    bool hasTexture = false;

    glm::mat4 modelM{1.0f};
    glm::mat4 lookatM{1.0f};
    glm::mat4 projectionM{1.0f};
    glm::mat4 viewportM{1.0f};
    
    glm::vec3 boundingBoxDimensions;
    glm::vec3 boundingBoxCenter;
    glm::vec3 lightPositionEyeCoordinates;

    // Helper methods
    void vertexProcessing(const PlotPixelCallback& plotPixel, VertexData& vertex);
    void rasterization(const PlotPixelCallback& plotPixel, const VertexData& vertex1, const VertexData& vertex2, const VertexData& vertex3, const glm::vec3& faceColor);
    void finalizeVertex(const PlotPixelCallback& plotPixel, VertexData& vertex);

    static VertexData interpolateVertex(const VertexData& v1, const VertexData& v2, float interpolationWeight);
    static std::vector<std::array<VertexData, 3>> clipTriangleAgainstPlane(glm::vec3 planePoint, glm::vec3 planeNormal, std::array<VertexData, 3>& inputTriangle);

    glm::vec3 fragmentProcessing(const FragmentData& fragmentData);
    static void drawLineBresenham(const PlotPixelCallback& plotPixel, const glm::vec3& p1, const glm::vec3& p2, float r, float g, float b);
    static glm::ivec4 calcBoundingBox(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int imageWidth, int imageHeight);
};