#pragma once
#include <string>

namespace DefaultParams {
    // Constants
    constexpr int IMAGE_WIDTH = 600;
    constexpr int IMAGE_HEIGHT = 600;
    constexpr int DEPTH_OF_RAY_TRACING = 6;

    // Ray Tracer default model
    inline const std::string RT_MODEL_FILE_NAME = "./RT_Models/ex_02___skybox_with_direction_labels.model";

    // Rasterizer default model
    inline const std::string RAST_MODEL_FILE_NAME = "./RAST_Models/models_with_textures/sheep.obj"; 
}