#pragma once

#include <vector>
#include <string>
#include <memory>
#include <random>
#include <functional>
#include <glm/glm.hpp>

#include "ExerciseEnum.h"
#include "ObjectModel.h"

class RasterizerWorld {
public:
    RasterizerWorld(int imageWidth, int imageHeight);

    bool load(const std::string& fileName);
    bool modelHasTexture() const;

    // Replacing IntBufferWrapper with standard callbacks
    using ClearImageCallback = std::function<void()>;
    using SetPixelCallback = std::function<void(int x, int y, const glm::vec3& color)>;

    void render(const ClearImageCallback& clearImage, const SetPixelCallback& setPixel);

    // type of rendering
    ProjectionTypeEnum projectionType;
    DisplayTypeEnum displayType;
    bool displayNormals = false;

    RasterizationExerciseEnum exercise;

    // camera location parameters
    glm::vec3 cameraPos;
    glm::vec3 cameraLookAtCenter;
    glm::vec3 cameraUp;
    float horizontalFOV;

    // transformation parameters
    float modelScale;

    // lighting parameters
    float lighting_Diffuse;
    float lighting_Specular;
    float lighting_Ambient;
    float lighting_sHininess;
    glm::vec3 lightPositionWorldCoordinates;
    
    std::vector<float> zBuffer;

    // random object
    std::mt19937 rand_engine{std::random_device{}()};
    
private:
    int imageWidth;
    int imageHeight;

    std::unique_ptr<ObjectModel> object1;
    
    int counter = 0;
    
    // growing location coordinates of object
    float objectX;
    float objectY;

    void clearZbuffer();
};