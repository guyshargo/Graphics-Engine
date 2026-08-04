#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <optional>
#include <vector>
#include "IntersectionResults.h"
#include "SavedParams.h"
#include "SphereTexture.h"

class Model;
class ErrorLogger;

class WorldModel {

    public:
        // Constructor
        WorldModel(int imageWidth, int imageHeight, float fovXdegree);
        ~WorldModel(); // Destructor to clean up resources

        // Sets the rendering parameters and loads the model from the specified file
        bool load(const std::string& filename);

        void setRenderingParams(int depthOfRayTracing);
        
        void setExercise(ExerciseEnum exercise);

        // Renders the color of a specific pixel in the image
        glm::vec3 renderPixel(int x, int y);

        // Calculates the normalized direction vector for a specific pixel
        static glm::vec3 calcPixelDirection(int x, int y, int imageWidth, int imageHeight, float fovXdegree);

        // Calculates the color of a point on a surface based on lighting parameters
        static glm::vec3 lightingEquation(glm::vec3 point,
                                          glm::vec3 pointNormal, 
                                          glm::vec3 lightPosition, 
                                          glm::vec3 kd, 
                                          glm::vec3 ks, 
                                          glm::vec3 ka, 
                                          float shininess);

        // Calculates the combined diffuse reflection coefficient (Kd) and texture color for a point on a sphere
        static glm::vec3 calcKdCombinedWithTexture(glm::vec3 intersectionPoint, 
                                                  glm::vec3 intersectedSphereCenter, 
                                                  const SphereTexture& intersectedSphereTexture, 
                                                  glm::vec3 intersectedSphereKd, 
                                                  float kTexture);

        // Determines if a point on a surface is in shadow relative to a light source
        static bool isPointInShadow(glm::vec3 lightLocation, glm::vec3 point, glm::vec3 pointNormal, const Model& model);

        // Calculates the intersection(s) between a ray and a sphere
        static std::optional<IntersectionResults> rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, 
                                                                    const ModelSphere& sphere);

        // Calculates the reflected light at an intersection point
        glm::vec3 calcReflectedLight(glm::vec3 incidentRayDirection,
                                     glm::vec3 intersectionPoint, 
                                     glm::vec3 intersectionNormal,
                                     const Model& model, 
                                     const SphereTexture& skyBoxImageSphereTexture, 
                                     int depthLevel) const;

        // Calculates the transmitted light at an intersection point
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
            std::unique_ptr<SphereTexture> skyBoxImageSphereTexture;
            
            // Performs ray tracing for a given ray
            glm::vec3 rayTracing(glm::vec3 incidentRayOrigin, glm::vec3 incidentRayDirection, const Model& model, 
                const SphereTexture& skyBoxImageSphereTexture, int depthLevel) const;

            // Finds the nearest intersection between a ray and a list of spheres
            static std::optional<IntersectionResults> rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, 
                const std::vector<ModelSphere>& spheres);
};