#pragma once
#include <magic_enum.hpp>

/**
 * @brief Defines the exact text labels used to switch between different rendering techniques, 
 *        mathematical exercises, and camera types within the engine's user interface.
 */
enum class EngineMode {
    RAY_TRACING,
    RASTERIZATION
};

enum class RayTracingDisplayTypeEnum {
    EX_2_Rays_calculation,
    EX_3_1_Intersection_One_sphere,
    EX_3_3_Intersection_List_of_spheres,
    EX_4_1_Lighting_Diffusive,
    EX_4_2_Lighting_Ambient,
    EX_4_3_Lighting_Specular,
    EX_5_Texture,
    EX_7_Reflection,
    EX_8_Transparency
};

enum class RasterizationDisplayTypeEnum {
    FACE_EDGES, 
    FACE_COLOR, 
    INTERPOlATED_VERTEX_COLOR, 
    LIGHTING_FLAT, 
    LIGHTING_GOURARD, 
    LIGHTING_PHONG, 
    TEXTURE, 
    TEXTURE_LIGHTING
};

enum class ProjectionTypeEnum {
    ORTHOGRAPHIC, 
    PERSPECTIVE
};