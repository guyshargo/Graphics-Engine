#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <optional>
#include "./Data/IntersectionResults.h"
#include "./Scene/ModelSphere.h"

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

namespace BVH {
    BoundingSphere computeBoundingSphere(const std::vector<const ModelSphere*>& spheres);
    std::unique_ptr<BVHNode> buildBVH(std::vector<const ModelSphere*> spheres, int depth);
    bool intersectBoundingSphere(const glm::vec3& rayStart, const glm::vec3& rayDirection, const BoundingSphere& bounds);
    std::optional<IntersectionResults> rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, const ModelSphere& sphere);
    std::optional<IntersectionResults> rayIntersectionBVH(const BVHNode* node, const glm::vec3& rayStart, const glm::vec3& rayDirection);
}