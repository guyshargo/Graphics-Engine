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

    // String literals
    inline const std::string MODEL_FILE_NAME = "./Models/ex_02___skybox_with_direction_labels.model";
    inline const std::string MODEL_OPEN_OBJ_PATH = "./Models/";
    inline const std::string SAVE_IMAGE_PATH = "./";
}