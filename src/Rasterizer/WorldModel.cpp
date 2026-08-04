#include "WorldModel.h"
#include "YourUtilities.h" 
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

// Constructor
WorldModel::WorldModel(int width, int height) : imageWidth(width), imageHeight(height) {
    zBuffer.resize(imageWidth * imageHeight, 1.0f);
}

bool WorldModel::load(const std::string& fileName) {
    object1 = std::make_unique<ObjectModel>(this, imageWidth, imageHeight);
    return object1 -> load(fileName);
}

bool WorldModel::modelHasTexture() const { return object1 -> objectHasTexture(); }

void WorldModel::render(const ClearImageCallback& clearImage, const SetPixelCallback& setPixel) {
    counter += 1;
    clearImage();
    clearZbuffer();
    object1 -> initTransformations();

    if (projectionType == ProjectionTypeEnum::ORTHOGRAPHIC) {
        glm::mat4 orthoM = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.0f, 100.0f);
        object1 -> setProjectionM(orthoM);
    }

    glm::mat4 viewportM = YourUtilities::createViewportMatrix(0.0f, 0.0f, imageWidth, imageHeight);
    object1 -> setViewportM(viewportM);

    glm::mat4 lookatM = glm::lookAt(cameraPos, cameraLookAtCenter, cameraUp);
    object1 -> setLookatM(lookatM);

    if (projectionType == ProjectionTypeEnum::PERSPECTIVE) {
        glm::mat4 perspectiveM = glm::perspective(glm::radians(30.0f), 1.0f, 1.0f, 100.0f);
        object1 -> setProjectionM(perspectiveM);
    }

    object1 -> render(setPixel);
}

void WorldModel::clearZbuffer() {
    std::fill(zBuffer.begin(), zBuffer.end(), 1.0f);
}