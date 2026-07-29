#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>

class Model;
class SphereTexture;
class ErrorLogger;

enum class ExerciseEnum {
    EX_2___Rays_calculation,
    EX_3_1_Intersection_One_sphere,
    EX_3_2_Intersection_One_sphere_with_color,
    EX_3_3_Intersection_List_of_spheres,
    EX_3_4_Intersection_Finding_the_nearest_sphere,
    EX_4_1_Lighting_Diffusive,
    EX_4_2_Lighting_Ambient,
    EX_4_3_Lighting_Specular,
    EX_5___Texture,
    EX_6___Shadow,
    EX_7___Reflection,
    EX_8___Transparency
};

class WorldModel {

    public:
        // Constructor
        WorldModel(int imageWidth, int imageHeight, float fovXdegree);

        bool loadModel(const std::string& filename);

        // Renders the color of a specific pixel in the image
        glm::vec3 renderPixel(int x, int y);

        // Calculates the normalized direction vector for a specific pixel
        glm::vec3 calcPixelDirection(int x, int y, int imageWidth, int imageHeight, float fovXdegree);



        private:
        int imageWidth;
        int imageHeight;
        float fovXdegree;
        static int depthOfRayTracing;
        std::unique_ptr<Model> loadedModel;
};