#include "WorldModel.h"
#include "Model.h"
#include <glm/gtc/constants.hpp> // For math constants
#include <glm/trigonometric.hpp> // For radians() and tan()

// Constructor implementation initializing variables
WorldModel::WorldModel(int imageWidth, int imageHeight, float fovXdegree)
    : imageWidth(imageWidth), imageHeight(imageHeight) {
}

glm::vec3 WorldModel::renderPixel(int x, int y) {
    glm::vec3 pixelDirection = calcPixelDirection(x, y, imageWidth, imageHeight, fovXdegree);
    glm::vec3 pixelColor(0.0f); // RAYTRACING LOGIC HERE
    return pixelColor;
}

glm::vec3 WorldModel::calcPixelDirection(int x, int y, int imageWidth, int imageHeight, float fovXdegree){
    // X axis
    float xLeft = -glm::tan(glm::radians(fovXdegree) / 2.0f);
    int pixelSpacesX = imageWidth - 1;
    float xDelta = (-xLeft * 2.0f) / static_cast<float>(pixelSpacesX);

    // Y axis
    float fovYdegree = fovXdegree/(static_cast<float>(imageWidth) / static_cast<float>(imageHeight));
    float yBottom = -glm::tan(glm::radians(fovYdegree) / 2.0f);
    int pixelSpacesY = imageHeight - 1;
    float yDelta = (-yBottom * 2.0f) / static_cast<float>(pixelSpacesY);

    // vector from eye to specific pixel in window
    glm::vec3 pixelDirection = glm::vec3(xLeft + xDelta * x, yBottom + yDelta * y, -1.0f);
    return glm::normalize(pixelDirection);
}

