#include "RasterizerWorld.h"
#include "./RasterUtils/TransformUtils.h" 
#include "DefaultParams.h"

#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

// Constructor
RasterizerWorld::RasterizerWorld(int width, int height) : imageWidth(width), imageHeight(height) {
    zBuffer.resize(imageWidth * imageHeight, 1.0f);
}

bool RasterizerWorld::load(const std::string& fileName) {
    object1 = std::make_unique<ObjectModel>(this, imageWidth, imageHeight);
    return object1 -> load(fileName);
}

bool RasterizerWorld::modelHasTexture() const { return object1 -> objectHasTexture(); }

void RasterizerWorld::render(const ClearImageCallback& clearImage, const SetPixelCallback& setPixel) {
    counter += 1;
    clearImage();
    clearZbuffer();
    object1 -> initTransformations();

    // Pivot Point Transformations
    glm::vec3 objectCenter = object1 -> getBoundingBoxCenter();
    glm::mat4 identity(1.0f);

    // Shift the object so its exact center sits at the world origin (0,0,0) to ensure it rotates uniformly
    glm::mat4 toOrigin = glm::translate(identity, -objectCenter);
    glm::mat4 backFromOrigin = glm::translate(identity, objectCenter);
    
    // Create Rotation matrices
    glm::mat4 rotX = glm::rotate(identity, glm::radians(objectRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(identity, glm::radians(objectRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(identity, glm::radians(objectRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotationM = rotZ * rotY * rotX;

    // Create Scale and World Translation matrices
    glm::mat4 scaleM = glm::scale(identity, objectScale);
    glm::mat4 worldPosM = glm::translate(identity, objectPosition);

    // Apply scale and rotation while centered, then move the object to its final requested world position
    glm::mat4 modelM = worldPosM * backFromOrigin * rotationM * scaleM * toOrigin;
    object1 -> setModelM(modelM);

    if (projectionType == ProjectionTypeEnum::ORTHOGRAPHIC) {
        glm::mat4 orthoM = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.0f, DefaultParams::PROJ_NEAR_PLANE);
        object1 -> setProjectionM(orthoM);
    }

    glm::mat4 viewportM = TransformUtils::createViewportMatrix(0.0f, 0.0f, imageWidth, imageHeight);
    object1 -> setViewportM(viewportM);

    glm::mat4 lookatM = glm::lookAt(cameraPos, cameraLookAtCenter, cameraUp);
    object1 -> setLookatM(lookatM);

    if (projectionType == ProjectionTypeEnum::PERSPECTIVE) {
        glm::mat4 perspectiveM = glm::perspective(glm::radians(DefaultParams::HORIZONTAL_FOV), 
                                                  DefaultParams::ASPECT_RATIO, 
                                                  DefaultParams::PROJ_NEAR_PLANE, 
                                                  DefaultParams::PROJ_FAR_PLANE);

        object1 -> setProjectionM(perspectiveM);
    }

    object1 -> render(setPixel);
}

void RasterizerWorld::clearZbuffer() {

    // Fill the depth buffer with the maximum possible distance so any new geometry draws over it
    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::max());
}

void RasterizerWorld::zoomCamera(float scrollDelta) {
    // Define how fast the zoom triggers per scroll tick
    float zoomSpeed = 0.5f; 
    
    // Calculate the forward direction vector
    glm::vec3 forward = glm::normalize(cameraLookAtCenter - cameraPos);
    
    // Move the camera position along the forward vector
    cameraPos += forward * (scrollDelta * zoomSpeed);
}

void RasterizerWorld::rotateCamera(float dx, float dy) {
    float sensitivity = 0.01f;
    
    // Get current directional vectors
    glm::vec3 forward = glm::normalize(cameraLookAtCenter - cameraPos);
    glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));
    
    // Horizontal rotation around the camera's current Up vector
    glm::mat4 yaw = glm::rotate(glm::mat4(1.0f), -dx * sensitivity, cameraUp);
    
    // Vertical rotation around the camera's current Right vector
    glm::mat4 pitch = glm::rotate(glm::mat4(1.0f), -dy * sensitivity, right);

    // Combine rotations
    glm::mat4 combinedRotation = yaw * pitch;
    
    // Apply rotation to the camera's position (relative to the target center)
    glm::vec4 relativePos = glm::vec4(cameraPos - cameraLookAtCenter, 1.0f);
    cameraPos = cameraLookAtCenter + glm::vec3(combinedRotation * relativePos);
    
    // Apply the exact same rotation to the cameraUp vector
    cameraUp = glm::normalize(glm::vec3(combinedRotation * glm::vec4(cameraUp, 0.0f)));
}

void RasterizerWorld::panCamera(float deltaX, float deltaY) {
    float panSpeed = 0.02f;

    // Calculate local right and up vectors
    glm::vec3 forward = glm::normalize(cameraLookAtCenter - cameraPos);
    glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));
    
    // Calculate the movement vector (invert deltaX/Y depending on drag preference)
    glm::vec3 movement = (right * -deltaX * panSpeed) + (up * deltaY * panSpeed);
    
    // Apply movement to both the camera position and the look-at target
    cameraPos += movement;
    cameraLookAtCenter += movement;
}