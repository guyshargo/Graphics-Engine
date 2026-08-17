#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include "./Data/IntersectionResults.h"
#include "SavedParams.h"
#include "./Textures/SphereTexture.h"
#include "./Acceleration/BVH.h"

class Model;

class RayTracerWorld {

    public:
        
        /**
         * @brief Initializes the rendering engine, setting the resolution and the viewing angle for the scene.
         * 
         * @param imageWidth The horizontal pixel count of the final image.
         * @param imageHeight The vertical pixel count of the final image.
         * @param fovXdegree The horizontal field of view angle in degrees.
         */
        RayTracerWorld(int imageWidth, int imageHeight, float fovXdegree);

        /**
         * @brief Destructor to clean up resources used by the RayTracerWorld
         */
        ~RayTracerWorld();


        /**
         * @brief Gets the currently loaded ray-tracing model
         *
         * @return A pointer to the loaded Model, or nullptr if no model has been loaded
         */
        Model* getModel() { return model.get(); }


        /**
         * @brief Controls how many times a light ray is allowed to bounce off shiny or transparent objects before stopping.
         * 
         * @param depthOfRayTracing The maximum number of reflection or refraction bounces.
         */
        void setDepthOfRayTracing(int depthOfRayTracing);

        /**
         * @brief Controls how many rays are fired per pixel to smooth out jagged edges.
         * 
         * @param antialiasingSamples The total number of offset rays to calculate per screen pixel.
         */
        void setAntialiasingSamples(int antialiasingSamples);

        /**
         * @brief Controls how many test rays are sent toward the light source to create blurry, realistic shadow edges.
         * 
         * @param softShadowSamples The total number of shadow-testing rays.
         */
        void setSoftShadowSamples(int softShadowSamples);

        /**
         * @brief Adjusts the blurriness of out-of-focus objects.
         * 
         * @param aperatureRadius The width of the simulated aperture opening.
         */
        void setAperatureRadius(float aperatureRadius);

        /**
         * @brief Sets the exact distance where objects appear perfectly sharp and in focus.
         * 
         * @param focalDistance The distance in 3D units from the camera to the focal plane.
         */
        void setFocalDistance(float focalDistance);
                
        /**
         * @brief Sets the current ray tracing exercise mode, which controls which engine features are active
         *
         * @param exercise The specific RayTracingExerciseEnum value to activate
         */
        void setExercise(RayTracingExerciseEnum exercise);

        
        /**
         * @brief Reads a scene file to spawn the 3D objects, sets up the sky background, and organizes the spatial boundaries.
         * 
         * @param filename The system path to the scene configuration file.
         * @return True if the scene and all assets successfully loaded, false otherwise.
         */
        bool load(const std::string& filename);


        /**
         * @brief Fires light rays through a specific screen coordinate and averages their colors to determine the final pixel output.
         * 
         * @param x The horizontal screen pixel coordinate.
         * @param y The vertical screen pixel coordinate.
         * @return The final averaged RGB color for the given pixel.
         */
        glm::vec3 renderPixel(int x, int y);


        /**
         * @brief Determines the exact 3D angle a light ray must travel to pass through a specific pixel on the screen.
         * 
         * @param x The horizontal screen pixel coordinate.
         * @param y The vertical screen pixel coordinate.
         * @param imageWidth The total width of the screen.
         * @param imageHeight The total height of the screen.
         * @param fovXdegree The horizontal field of view angle.
         * @return The calculated 3D trajectory vector for the ray.
         */
        static glm::vec3 calcPixelDirection(float x, float y, int imageWidth, int imageHeight, float fovXdegree);
                                                                    
        /**
         * @brief Blends the diffuse reflection coefficient (Kd) of a material with the sampled texture color for a specific point on a sphere
         *
         * @param intersectionPoint The exact point in 3D space where the ray hit the sphere
         * @param intersectedSphereCenter The center coordinate of the intersected sphere
         * @param intersectedSphereTexture The SphereTexture object applied to the sphere
         * @param intersectedSphereKd The base diffuse color (Kd) of the sphere's material
         * @param kTexture The interpolation weight determining how much the texture influences the final color
         * @return A glm::vec3 representing the combined diffuse and texture color
         */
        static glm::vec3 calcKdCombinedWithTexture(glm::vec3 intersectionPoint, 
                                                  glm::vec3 intersectedSphereCenter, 
                                                  const SphereTexture& intersectedSphereTexture, 
                                                  glm::vec3 intersectedSphereKd, 
                                                  float kTexture);
        
        /**
         * @brief Tests if other objects are blocking the light source from illuminating a specific point, darkening it if true.
         * 
         * @param lightLocation The 3D coordinates of the light source.
         * @param point The specific surface coordinates being tested for illumination.
         * @param pointNormal The direction the surface is facing, used to offset the testing ray.
         * @param model The full 3D scene data.
         * @return A fraction between 0.0 and 1.0 representing how completely the light is blocked.
         */
        float isPointInShadow(glm::vec3 lightLocation, glm::vec3 point, glm::vec3 pointNormal, const Model& model) const;

        /**
         * @brief Calculates the color seen on a shiny surface by bouncing the light ray off it and seeing what object it hits next.
         * 
         * @param incidentRayDirection The trajectory of the light ray before hitting the surface.
         * @param intersectionPoint The exact 3D coordinates where the ray hit the surface.
         * @param intersectionNormal The direction the surface is facing.
         * @param model The full 3D scene data.
         * @param skyBoxImageSphereTexture The background panoramic image.
         * @param depthLevel The current tracking depth of the bouncing ray.
         * @return The RGB color retrieved from the bounced ray.
         */
        glm::vec3 calcReflectedLight(glm::vec3 incidentRayDirection,
                                     glm::vec3 intersectionPoint, 
                                     glm::vec3 intersectionNormal,
                                     const Model& model, 
                                     const SphereTexture& skyBoxImageSphereTexture, 
                                     int depthLevel) const;

        
        /**
         * @brief Calculates the distorted colors seen through clear objects by bending the light ray as it passes through them.
         * 
         * @param incidentRayDirection The trajectory of the light ray before hitting the clear object.
         * @param intersectionPoint The exact 3D coordinates where the ray entered or exited the object.
         * @param intersectionNormal The direction the surface is facing.
         * @param intersectionFromOutsideOfSphere True if the ray is entering the object, false if it is exiting.
         * @param refractiveIndexIntersectedSphere The material density determining how the light bends.
         * @param model The full 3D scene data.
         * @param skyBoxImageSphereTexture The background panoramic image.
         * @param depthLevel The current tracking depth of the passing ray.
         * @return The RGB color retrieved from the bent ray.
         */
        glm::vec3 calcTransmissionLight(glm::vec3 incidentRayDirection, 
                                        glm::vec3 intersectionPoint, 
                                        glm::vec3 intersectionNormal,
                                        bool intersectionFromOutsideOfSphere, 
                                        float refractiveIndexIntersectedSphere, 
                                        const Model& model, 
                                        const SphereTexture& skyBoxImageSphereTexture, 
                                        int depthLevel) const;


        private:
            int imageWidth;
            int imageHeight;
            float fovXdegree;
            SavedParams params;

            std::unique_ptr<Model> model;
            std::unique_ptr<BVHNode> bvhRoot;
            std::unique_ptr<SphereTexture> skyBoxImageSphereTexture;
            
            /**
             * @brief The core rendering loop that shoots a single ray into the scene 
             *        and calculates its color based on direct light, shadows, reflections, and transparency.
             * 
             * @param incidentRayOrigin The starting 3D coordinates of the ray.
             * @param incidentRayDirection The trajectory of the ray.
             * @param model The full 3D scene data.
             * @param skyBoxImageSphereTexture The background panoramic image.
             * @param depthLevel The current tracking depth of the bouncing ray.
             * @return The final calculated RGB color for the ray.
             */
            glm::vec3 rayTracing(glm::vec3 incidentRayOrigin, glm::vec3 incidentRayDirection, const Model& model, 
                const SphereTexture& skyBoxImageSphereTexture, int depthLevel) const;

};