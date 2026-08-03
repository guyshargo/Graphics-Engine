#pragma once

enum class ExerciseEnum {
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
    EX_8_Transparency,
    COUNT // This is a sentinel value to represent the number of exercises
};

// Returns a human-readable exercise name for UI display
inline const char* GetExerciseDescription(ExerciseEnum ex) {
    static const char* descriptions[] = {
        "Exercise 2 - Rays calculation",
        "Exercise 3.1 - Intersection - One sphere",
        "Exercise 3.2 - Intersection - One sphere with color",
        "Exercise 3.3 - Intersection List of spheres",
        "Exercise 3.4 - Intersection - Finding the nearest sphere",
        "Exercise 4.1 - Lighting - Diffusive",
        "Exercise 4.2 - Lighting - Ambient",
        "Exercise 4.3 - Lighting - Specular",
        "Exercise 5 - Texture",
        "Exercise 6 - Shadow",
        "Exercise 7 - Reflection",
        "Exercise 8 - Transparency"
    };

    int index = static_cast<int>(ex);
    if (index >= 0 && index < static_cast<int>(ExerciseEnum::COUNT)) {
        return descriptions[index];
    }
    return "Unknown Exercise";
}

// returns the total number of exercises for UI loops
inline int GetExerciseCount() {
    return static_cast<int>(ExerciseEnum::COUNT);
}