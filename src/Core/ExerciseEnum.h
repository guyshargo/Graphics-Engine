#pragma once
#include <magic_enum.hpp>

enum class EngineMode {
    RAY_TRACING,
    RASTERIZATION
};

enum class RayTracingExerciseEnum {
    EX_2_Rays_calculation,
    EX_3_1_Intersection_One_sphere,
    EX_3_2_Intersection_One_sphere_with_color,
    EX_3_3_Intersection_List_of_spheres,
    EX_3_4_Intersection_Finding_the_nearest_sphere,
    EX_4_1_Lighting_Diffusive,
    EX_4_2_Lighting_Ambient,
    EX_4_3_Lighting_Specular,
    EX_5_Texture,
    EX_6_Shadow,
    EX_7_Reflection,
    EX_8_Transparency
};

enum class RasterizationExerciseEnum {
    EX_1_Lines_rasterization,
    EX_2_Triangles_rasterization,
    EX_4_Orthograpic_projection_and_viewport,
    EX_5_lookat,
    EX_6_Perspective_projection,
    EX_7_Vertex_color_interpolation,
    EX_8_Z_buffer,
    EX_9_Lighting,
    EX_10_Texture
};

enum class DisplayTypeEnum {
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