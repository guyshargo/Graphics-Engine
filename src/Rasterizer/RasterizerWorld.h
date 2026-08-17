#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

#include "ExerciseEnum.h"
#include "./Scene/ObjectModel.h"

/**
 * @brief The main engine coordinator that manages the camera, tracks depth to ensure 
 *        closer objects block further ones, and triggers the rendering pipeline for the scene.
 */
class RasterizerWorld {
public:
    /**
     * @brief Configures the main rendering screen dimensions and prepares the depth buffer.
     */
    RasterizerWorld(int imageWidth, int imageHeight);

    /**
     * @brief Directs the object model to read and load a 3D scene file from the hard drive.
     */
    bool load(const std::string& fileName);

    /**
     * @brief Checks if the currently loaded 3D model has an associated image texture.
     * 
     * @return True if a texture is loaded and valid, false otherwise.
     */
    bool modelHasTexture() const;

    /**
     * @brief Defines the expected function signature for clearing the output image buffer.
     */
    using ClearImageCallback = std::function<void()>;

    /**
     * @brief Defines the expected function signature for plotting a single RGB color to a specific screen coordinate.
     * 
     * @param x The horizontal screen pixel coordinate.
     * @param y The vertical screen pixel coordinate.
     * @param color The final computed RGB color for the pixel.
     */
    using SetPixelCallback = std::function<void(int x, int y, const glm::vec3& color)>;

    /**
     * @brief Clears the previous frame, calculates the camera's current perspective, 
     *        and commands the loaded 3D models to draw themselves onto the screen.
     * 
     * @param clearImage The function executed to wipe the screen buffer clean.
     * @param setPixel The function executed to draw the final calculated colors to the screen buffer.
     */
    void render(const ClearImageCallback& clearImage, const SetPixelCallback& setPixel);

    // display options
    ProjectionTypeEnum projectionType;
    DisplayTypeEnum displayType;
    bool displayNormals = false;

    // camera location parameters
    glm::vec3 cameraPos;
    glm::vec3 cameraLookAtCenter;
    glm::vec3 cameraUp;
    float horizontalFOV;

    // transformation parameters
    glm::vec3 objectPosition{0.0f, 0.0f, 0.0f};
    glm::vec3 objectRotation{0.0f, 0.0f, 0.0f}; // Stored in degrees
    glm::vec3 objectScale{1.0f, 1.0f, 1.0f};

    // lighting parameters
    float lighting_Diffuse;
    float lighting_Specular;
    float lighting_Ambient;
    float lighting_sHininess;
    glm::vec3 lightPositionWorldCoordinates;
    
    // depth buffer
    std::vector<float> zBuffer;

    /**
     * @brief Pushes the camera forward or backward along its current viewing trajectory.
     * 
     * @param scrollDelta The mouse wheel input determining the zoom direction and speed.
     */
    void zoomCamera(float scrollDelta);

    /**
     * @brief Orbits the camera around the target center point by adjusting its pitch and yaw.
     * 
     * @param deltaX The horizontal mouse movement used to calculate the yaw rotation.
     * @param deltaY The vertical mouse movement used to calculate the pitch rotation.
     */
    void rotateCamera(float deltaX, float deltaY);

    /**
     * @brief Slides both the camera and its viewing target horizontally or vertically across the scene.
     * 
     * @param deltaX The horizontal mouse movement used to shift the camera on its local X axis.
     * @param deltaY The vertical mouse movement used to shift the camera on its local Y axis.
     */
    void panCamera(float deltaX, float deltaY);
    
private:
    int imageWidth;
    int imageHeight;

    std::unique_ptr<ObjectModel> object1;
    
    int counter = 0;
    
    // growing location coordinates of object
    float objectX;
    float objectY;

    /**
     * @brief Resets the depth tracking array to the maximum possible distance, preparing the engine for a new frame.
     */
    void clearZbuffer();
};