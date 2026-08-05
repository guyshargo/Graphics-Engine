#pragma once
#include <string>

namespace DefaultParams {
    // Constants
    constexpr int IMAGE_WIDTH = 600;
    constexpr int IMAGE_HEIGHT = 600;
    constexpr int IMAGE_UPDATE_INTERVAL_IN_MS = 30;
    constexpr int LABELS_UPDATE_INTERVAL_IN_MS = 250;
    constexpr int SIZE_OF_UPPER_INTERFACE_ROW_IN_PIXELS = 40;
    constexpr int DEPTH_OF_RAY_TRACING = 6;

    // String literals - Ray Tracer
    inline const std::string RT_MODEL_FILE_NAME = "./RT_Models/ex_02___skybox_with_direction_labels.model";
    inline const std::string RT_MODEL_OPEN_OBJ_PATH = "./RT_Models/";

    // String literals - Rasterizer
    inline const std::string RAST_MODEL_FILE_NAME = "./RAST_Models/models_with_textures/sheep.obj"; 
    inline const std::string RAST_MODEL_OPEN_OBJ_PATH = "./RAST_Models/";

    inline const std::string SAVE_IMAGE_PATH = "./";
}