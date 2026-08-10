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

/**
 * @brief Spherical bounds used to enclose one or more model spheres in the BVH
 */
struct BoundingSphere {
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    float radius{0.0f};
};

/**
 * @brief Node in the bounding volume hierarchy used to accelerate ray intersections
 */
struct BVHNode {
    BoundingSphere bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    
    //Model spheres stored by a leaf node. internal nodes leave this empty
    std::vector<const ModelSphere*> leafSpheres; 

    /**
     * @brief Determines whether this node is a leaf
     * @return true when the node has no child nodes, otherwise false
     */
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

class RayTracerWorld {

    public:
        
        /**
         * @brief Initializes the RayTracerWorld with the specified image dimensions and field of view
         *
         * @param imageWidth The width of the image to be rendered
         * @param imageHeight The height of the image to be rendered
         * @param fovXdegree The horizontal field of view in degrees
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
         * @brief Sets the rendering parameters used by the ray tracer
         *
         * @param depthOfRayTracing The maximum number of recursive ray bounces allowed
         * @param antialiasingSamples The number of randomly sampled rays used per pixel
         * @param softShadowSamples The number of sampled light rays used to estimate soft shadows
         * @param exercise The ray-tracing exercise that determines which rendering features are enabled
         */
        void setDepthOfRayTracing(int depthOfRayTracing);

        void setAntialiasingSamples(int antialiasingSamples);

        void setSoftShadowSamples(int softShadowSamples);
                
        /**
         * @brief Sets the current ray tracing exercise mode, which controls which engine features are active
         *
         * @param exercise The specific RayTracingExerciseEnum value to activate
         */
        void setExercise(RayTracingExerciseEnum exercise);

        
        /**
         * @brief Loads the model and skybox texture from the specified file, and initializes the Bounding Volume Hierarchy (BVH) tree
         *
         * @param filename The path to the .model file to be loaded
         * @return true if the model, texture, and BVH tree loaded and initialized successfully; false otherwise
         */
        bool load(const std::string& filename);


        /**
         * @brief Renders and calculates the color of a pixel using multiple randomly sampled rays 
         * for stochastic antialiasing
         *
         * @param x The horizontal coordinate of the pixel on the screen
         * @param y The vertical coordinate of the pixel on the screen
         * @return A glm::vec3 representing the averaged RGB color of the sampled rays
         */
        glm::vec3 renderPixel(int x, int y);


        /**
         * @brief Calculates the normalized direction vector from the camera eye to a point on the view plane
         *
         * Fractional coordinates are supported so to generate subpixel samples for antialiasing.
         *
         * @param x The horizontal coordinate of the target pixel
         * @param y The vertical coordinate of the target pixel
         * @param imageWidth The total width of the image
         * @param imageHeight The total height of the image
         * @param fovXdegree The horizontal field of view in degrees
         * @return A normalized glm::vec3 representing the ray's direction in 3D space
         */
        static glm::vec3 calcPixelDirection(float x, float y, int imageWidth, int imageHeight, float fovXdegree);


        /**
         * @brief Calculates the exact intersection point between a ray and a single mathematical sphere
         *
         * @param rayStart The origin point of the ray
         * @param rayDirection The normalized direction vector of the ray
         * @param sphere The target ModelSphere to test for intersection
         * @return An std::optional containing IntersectionResults if a hit occurs, or std::nullopt if the ray misses completely
         */
        static std::optional<IntersectionResults> rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, 
                                                                    const ModelSphere& sphere);

        
                                                                    
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
         * @brief Determines if a specific surface point is occluded from the light source by checking the BVH tree for collisions along the shadow ray
         *
         * @param lightLocation The 3D coordinates of the light source
         * @param point The surface point to test for shadows
         * @param pointNormal The surface normal at the test point, used to offset the ray to prevent self-shadowing
         * @param model The scene model
         * @return true if the point is obstructed from the light (in shadow), false otherwise
         */
        float isPointInShadow(glm::vec3 lightLocation, glm::vec3 point, glm::vec3 pointNormal, const Model& model) const;



        /**
         * @brief Calculates the color contribution from reflected light by bouncing the incident ray across the surface normal and continuing the ray tracing process
         *
         * @param incidentRayDirection The normalized direction vector of the incoming ray
         * @param intersectionPoint The point on the surface where the reflection occurs
         * @param intersectionNormal The surface normal at the point of reflection
         * @param model The scene model containing all geometry
         * @param skyBoxImageSphereTexture The background environment map
         * @param depthLevel The current recursion depth
         * @return A glm::vec3 representing the RGB color of the reflected environment
         */
        glm::vec3 calcReflectedLight(glm::vec3 incidentRayDirection,
                                     glm::vec3 intersectionPoint, 
                                     glm::vec3 intersectionNormal,
                                     const Model& model, 
                                     const SphereTexture& skyBoxImageSphereTexture, 
                                     int depthLevel) const;

        
        /**
         * @brief Calculates the color contribution from light transmitting (refracting) through a transparent material
         * Falls back to computing reflected light if the refraction results in total internal reflection
         *
         * @param incidentRayDirection The normalized direction vector of the incoming ray
         * @param intersectionPoint The exact point where the ray enters or exits the material
         * @param intersectionNormal The surface normal at the intersection point
         * @param intersectionFromOutsideOfSphere A flag indicating if the ray is entering the sphere (true) or exiting it (false)
         * @param refractiveIndexIntersectedSphere The index of refraction for the intersected material
         * @param model The scene model containing all geometry
         * @param skyBoxImageSphereTexture The background environment map
         * @param depthLevel The current recursion depth
         * @return A glm::vec3 representing the RGB color of the transmitted light
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
             * @brief Performs recursive ray tracing to determine the color of a ray intersecting with the scene
             * Evaluates direct lighting, shadows, reflection, and transmission
             *
             * @param incidentRayOrigin The starting point of the ray in 3D space
             * @param incidentRayDirection The normalized direction vector of the ray
             * @param model The scene model containing spheres, materials, and lights
             * @param skyBoxImageSphereTexture The background texture used when a ray misses all geometry
             * @param depthLevel The current recursion depth of the ray
             * @return A glm::vec3 representing the accumulated RGB color for this ray path
             */
            glm::vec3 rayTracing(glm::vec3 incidentRayOrigin, glm::vec3 incidentRayDirection, const Model& model, 
                const SphereTexture& skyBoxImageSphereTexture, int depthLevel) const;


            /**
             * @brief Calculates a bounding sphere that fully encloses a provided collection of ModelSpheres
             *
             * @param spheres A vector of pointers to the ModelSphere objects to be enclosed
             * @return A BoundingSphere struct containing the centroid and the maximum radius required to encompass all given spheres
             */
            BoundingSphere computeBoundingSphere(const std::vector<const ModelSphere*>& spheres) const;


            /**
             * @brief Recursively partitions a collection of spheres to build a Bounding Volume Hierarchy (BVH) tree, alternating the sorting axis based on depth
             *
             * @param spheres A vector of pointers to the ModelSpheres to be organized into the tree
             * @param depth The current depth in the tree construction, used to determine the splitting axis (X, Y, or Z)
             * @return A unique_ptr to the constructed BVHNode serving as the root for this subset of geometry
             */
            std::unique_ptr<BVHNode> buildBVH(std::vector<const ModelSphere*> spheres, int depth);


            /**
             * @brief Performs a fast boolean intersection test between a ray and a bounding sphere to determine if the ray traverses this spatial region
             *
             * @param rayStart The origin point of the ray
             * @param rayDirection The normalized direction vector of the ray
             * @param bounds The BoundingSphere to test against
             * @return true if the ray intersects the bounding sphere; false if it misses completely
             */
            bool intersectBoundingSphere(const glm::vec3& rayStart, const glm::vec3& rayDirection, const BoundingSphere& bounds) const;


            /**
             * @brief Traverses the Bounding Volume Hierarchy (BVH) to find the closest intersection between a ray and the scene geometry
             * Sub-trees are entirely skipped if their bounding sphere is not intersected by the ray
             *
             * @param node The current BVHNode being evaluated
             * @param rayStart The origin point of the ray
             * @param rayDirection The normalized direction vector of the ray
             * @return An std::optional containing IntersectionResults for the closest hit, or std::nullopt if no geometry is intersected
             */
            std::optional<IntersectionResults> rayIntersectionBVH(const BVHNode* node, const glm::vec3& rayStart, const glm::vec3& rayDirection) const;
};