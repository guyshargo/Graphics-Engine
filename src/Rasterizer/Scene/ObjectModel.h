#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "./Data/PipelineData.h"
#include "ExerciseEnum.h"
#include "OBJLoader.h"
#include "./Textures/Texture2D.h"

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

    void render(const PlotPixelCallback& plotPixel);

private:
    RasterizerWorld* rasterizerWorld;
    int imageWidth;
    int imageHeight;

    std::vector<VertexData> verticesData;
    std::vector<TriangleFace> faces;

    std::unique_ptr<Texture2D> texture;

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

    int clipTriangleAgainstPlane(glm::vec3 planePoint, glm::vec3 planeNormal, 
                             const std::array<VertexData, 3>& inputTriangle, 
                             std::array<std::array<VertexData, 3>, 2>& outTriangles);
                             
    glm::vec3 fragmentProcessing(const FragmentData& fragmentData);
    static glm::ivec4 calcBoundingBox(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int imageWidth, int imageHeight);
};