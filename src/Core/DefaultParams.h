#pragma once
#include <string>

namespace DefaultParams {
    
    constexpr int IMAGE_WIDTH = 600;
    constexpr int IMAGE_HEIGHT = 600;

    // --- Ray Tracer default model ---
    inline const std::string RT_MODEL_FILE_NAME = "../../RT_Models/ex_02___skybox_with_direction_labels.model";
    constexpr int DEPTH_OF_RAY_TRACING = 6;
    inline int ANTIALIASING_SAMPLES = 16;
    inline int SOFT_SHADOW_SAMPLES = 2;

    // Trace multiple random samples within the pixel area for antialiasing
    inline float MIN_SUBPIXELS_RANGE = -0.5f;
    inline float MAX_SUBPIXELS_RANGE = 0.5f;

    // Blur intensity
    constexpr float APERATURE_RADIUS = 0.1f;
    // Focus plane
    constexpr float FOCAL_DISTANCE = 5.0f;

    // --- Rasterizer default model ---
    inline const std::string RAST_MODEL_FILE_NAME = "../../RAST_Models/models_with_textures/sheep.obj";

    constexpr float INITIAL_CAMERA_DISTANCE_FROM_AXIS_CENTER = 5.0f;
    constexpr float CAMERA_MAX_VERTICAL_ANGLE = 60.0f;

    constexpr float CAMERA_RADIUS = INITIAL_CAMERA_DISTANCE_FROM_AXIS_CENTER;
    inline const glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, CAMERA_RADIUS);
    inline const glm::vec3 cameraLookAtCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    inline const glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    constexpr float CAMERA_ANGLE_HORIZONTAL = 270.0f;
    constexpr float CAMERA_ANGLE_VERTICAL = 0.0f;
    constexpr float HORIZONTAL_FOV = 30.0f;
    constexpr float MODEL_SCALE = 1.0f;
    constexpr float PROJ_NEAR_PLANE = 1.0f;
    constexpr float PROJ_FAR_PLANE = 100.0f;
    constexpr float ASPECT_RATIO = static_cast<float>(IMAGE_WIDTH) / static_cast<float>(IMAGE_HEIGHT);

    constexpr float LIGHTING_DIFFUSE = 0.75f;
    constexpr float LIGHTING_SPECULAR = 0.2f;
    constexpr float LIGHTING_AMBIENT = 0.4f;
    constexpr float LIGHTING_SHININESS = 40.0f;
    inline const glm::vec3 lightPosition = glm::vec3(10.0f, 10.0f, 10.0f);

}