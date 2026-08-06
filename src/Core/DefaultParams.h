#pragma once
#include <string>

namespace DefaultParams {
    
    constexpr int IMAGE_WIDTH = 600;
    constexpr int IMAGE_HEIGHT = 600;

    // Ray Tracer default model
    inline const std::string RT_MODEL_FILE_NAME = "../../RT_Models/ex_02___skybox_with_direction_labels.model";
    constexpr int DEPTH_OF_RAY_TRACING = 6;

    // Rasterizer default model
    inline const std::string RAST_MODEL_FILE_NAME = "../../RAST_Models/models_with_textures/sheep.obj";

    constexpr float INITIAL_CAMERA_DISTANCE_FROM_AXIS_CENTER = 5.0f;
    constexpr float CAMERA_MAX_VERTICAL_ANGLE = 60.0f;

    constexpr float cameraRadius = INITIAL_CAMERA_DISTANCE_FROM_AXIS_CENTER;
    inline const glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, cameraRadius);
    inline const glm::vec3 cameraLookAtCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    inline const glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    constexpr float cameraAngleHorizontal = 270.0f;
    constexpr float cameraAngleVertical = 0.0f;
    constexpr float horizontalFOV = 45.0f;
    constexpr float modelScale = 1.0f;

    constexpr float lighting_Diffuse = 0.75f;
    constexpr float lighting_Specular = 0.2f;
    constexpr float lighting_Ambient = 0.4f;
    constexpr float lighting_sHininess = 40.0f;
    inline const glm::vec3 lightPosition = glm::vec3(10.0f, 10.0f, 10.0f);

}