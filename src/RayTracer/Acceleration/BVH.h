#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <optional>
#include "./Data/IntersectionResults.h"
#include "./Scene/ModelSphere.h"

/**
 * @brief Defines an invisible 3D boundary that groups multiple objects together, 
 *        allowing the engine to immediately skip calculating light rays for entire 
 *        sections of the scene if the ray misses the boundary.
 */
struct BoundingSphere {
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    float radius{0.0f};
};

/**
 * @brief A branching structure that organizes the 3D scene into smaller and smaller 
 *        bounding groups, drastically speeding up the rendering process by eliminating 
 *        unnecessary collision checks.
 */
struct BVHNode {
    BoundingSphere bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    
    // The spheres stored at the very end of a branching path
    std::vector<const ModelSphere*> leafSpheres; 

    /**
     * @brief Checks if this specific group contains spheres rather than more subgroups.
     * @return True if the group holds spheres, false if it branches into smaller groups.
     */
    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

namespace BVH {

    /**
     * @brief Calculates the smallest possible invisible sphere required to completely enclose a given list of objects.
     * 
     * @param spheres The list of 3D objects to enclose.
     * @return The calculated spherical boundary encompassing all provided objects.
     */
    BoundingSphere computeBoundingSphere(const std::vector<const ModelSphere*>& spheres);

    /**
     * @brief Organizes a list of objects into a spatial hierarchy by splitting them into left and right groups based on their 3D locations.
     * 
     * @param spheres The full list of 3D objects to sort into the structure.
     * @param depth The current recursion step, used to alternate the splitting axis (X, Y, or Z).
     * @return The top-level branching node containing the organized scene.
     */
    std::unique_ptr<BVHNode> buildBVH(std::vector<const ModelSphere*> spheres, int depth);

    /**
     * @brief Quickly checks if a light ray passes through an invisible bounding group.
     * 
     * @param rayStart The starting 3D coordinates of the light ray.
     * @param rayDirection The trajectory of the light ray.
     * @param bounds The spherical boundary to test against.
     * @return True if the ray enters the boundary, false if it completely misses.
     */
    bool intersectBoundingSphere(const glm::vec3& rayStart, const glm::vec3& rayDirection, const BoundingSphere& bounds);

    /**
     * @brief Calculates the precise mathematical collision between a light ray and a single round object.
     * 
     * @param rayStart The starting 3D coordinates of the light ray.
     * @param rayDirection The trajectory of the light ray.
     * @param sphere The specific 3D object to test against.
     * @return The exact collision data if the ray hits, or an empty result if it misses.
     */
    std::optional<IntersectionResults> rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, const ModelSphere& sphere);

    /**
     * @brief Traverses the organized spatial groups to find the absolute closest object the light ray hits.
     * 
     * @param node The current branching group being checked.
     * @param rayStart The starting 3D coordinates of the light ray.
     * @param rayDirection The trajectory of the light ray.
     * @return The exact collision data for the nearest hit object, or an empty result if the ray exits the scene.
     */
    std::optional<IntersectionResults> rayIntersectionBVH(const BVHNode* node, const glm::vec3& rayStart, const glm::vec3& rayDirection);
}