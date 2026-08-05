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

    using SetPixelCallback = std::function<void(int x, int y, const glm::vec3& color)>;
    using TextureGetPixel = std::function<glm::vec3(int x, int y)>;
    using TextureGetDimension = std::function<int()>;
    
    void setTextureCallbacks(TextureGetPixel getPixel, TextureGetDimension getWidth, TextureGetDimension getHeight);
    void render(const SetPixelCallback& setPixel);

    static RasterizationExerciseEnum exercise;

private:
    RasterizerWorld* rasterizerWorld;
    int imageWidth;
    int imageHeight;

    std::vector<VertexData> verticesData;
    std::vector<TriangleFace> faces;
    
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
    void vertexProcessing(const SetPixelCallback& setPixel, VertexData& vertex);
    void transformNormalFromObjectCoordToEyeCoordAndDrawIt(const SetPixelCallback& setPixel, VertexData& vertex);
    void rasterization(const SetPixelCallback& setPixel, const VertexData& vertex1, const VertexData& vertex2, const VertexData& vertex3, const glm::vec3& faceColor);
    
    glm::vec3 fragmentProcessing(const FragmentData& fragmentData);

    static void drawLineDDA(const SetPixelCallback& setPixel, const glm::vec3& p1, const glm::vec3& p2, float r, float g, float b);
    static glm::ivec4 calcBoundingBox(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int imageWidth, int imageHeight);
    
    float lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, float Kd, float Ks, float Ka, float shininess);
    static glm::vec3 lightingEquation(const glm::vec3& point, const glm::vec3& pointNormal, const glm::vec3& lightPos, const glm::vec3& Kd, const glm::vec3& Ks, const glm::vec3& Ka, float shininess);
};