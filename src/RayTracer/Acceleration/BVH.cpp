#include "BVH.h"
#include <algorithm>
#include <cmath>

namespace BVH {

    BoundingSphere computeBoundingSphere(const std::vector<const ModelSphere*>& spheres) {
        if (spheres.empty()) return {};
        if (spheres.size() == 1) return {spheres[0]->center, spheres[0]->radius};

        // Calculate the centroid of all spheres
        glm::vec3 center(0.0f);
        for (const auto* s : spheres) {
            center += s->center;
        }
        center /= static_cast<float>(spheres.size());

        // Find the maximum distance to any sphere's edge to determine the radius
        float maxRadius = 0.0f;
        for (const auto* s : spheres) {
            float distanceToEdge = glm::length(s->center - center) + s->radius;
            if (distanceToEdge > maxRadius) {
                maxRadius = distanceToEdge;
            }
        }

        return {center, maxRadius};
    }

    std::unique_ptr<BVHNode> buildBVH(std::vector<const ModelSphere*> spheres, int depth) {
        auto node = std::make_unique<BVHNode>();
        node->bounds = computeBoundingSphere(spheres);

        // Base Case: If the group is small enough, it becomes a leaf node
        if (spheres.size() <= 2) {
            node->leafSpheres = std::move(spheres);
            return node;
        }

        // Recursive Step: Sort spheres along an alternating axis to partition space
        int axis = depth % 3; // 0 = X, 1 = Y, 2 = Z
        std::sort(spheres.begin(), spheres.end(), [axis](const ModelSphere* a, const ModelSphere* b) {
            return a->center[axis] < b->center[axis];
        });

        // Split the spheres in half
        auto mid = spheres.begin() + spheres.size() / 2;
        std::vector<const ModelSphere*> leftSpheres(spheres.begin(), mid);
        std::vector<const ModelSphere*> rightSpheres(mid, spheres.end());

        // Recursively build children
        node->left = buildBVH(leftSpheres, depth + 1);
        node->right = buildBVH(rightSpheres, depth + 1);

        return node;
    }

    bool intersectBoundingSphere(const glm::vec3& rayStart, const glm::vec3& rayDirection, const BoundingSphere& bounds) {
        float tm = glm::dot(rayDirection, bounds.center - rayStart);
        glm::vec3 pm = rayStart + tm * rayDirection;
        float pmDistance = glm::length(pm - bounds.center);

        if (pmDistance > bounds.radius) {
            return false;
        }

        float dt = glm::sqrt(bounds.radius * bounds.radius - pmDistance * pmDistance);
        if (tm < -dt) {
            return false;
        }

        return true;
    }

    std::optional<IntersectionResults> rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, const ModelSphere& sphere) {
        glm::vec3 sphereCenter = sphere.center;
        float sphereRadius = sphere.radius;

        // Distance from ray start to closest point in ray from sphere center (pm)
        float tm = glm::dot(rayDirection, sphereCenter - rayStart);

        // closest point in ray from sphere center
        glm::vec3 pm = rayStart + tm * rayDirection;

        // distance from center to ray
        float pmDistance = glm::length(pm - sphereCenter);

        // If sphere radius is smaller than distance to closest point from center on the ray
        // The ray misses the sphere
        if (pmDistance > sphereRadius) {
            return std::nullopt;
        }

        // distance on ray from p1 to point which is closest to center
        float dt = glm::sqrt(sphereRadius * sphereRadius - pmDistance * pmDistance);

        // If the intersection points are behind the ray's starting point
        // The ray starts after the sphere
        if (tm < -dt) {
            return std::nullopt;
        }

        // If the ray starts inside the sphere
        if (tm < dt) {
            glm::vec3 firstIntersectionPoint = pm + dt * rayDirection;
            glm::vec3 normal = glm::normalize(sphereCenter - firstIntersectionPoint); // Normal points inside the sphere
            return IntersectionResults{true, 
                firstIntersectionPoint, 
                normal,
                false, 
                &sphere};
        }

        // If the ray starts before the sphere
        else {
            glm::vec3 firstIntersectionPoint = pm - dt * rayDirection;
            glm::vec3 normal = glm::normalize(firstIntersectionPoint - sphereCenter);
            return IntersectionResults{true, 
                firstIntersectionPoint, 
                normal,
                true, 
                &sphere};
        }

        return std::nullopt;
    }

    std::optional<IntersectionResults> rayIntersectionBVH(const BVHNode* node, const glm::vec3& rayStart, const glm::vec3& rayDirection) {
        // If the ray completely misses this node's bounding sphere, skip it
        if (!node || !intersectBoundingSphere(rayStart, rayDirection, node->bounds)) {
            return std::nullopt;
        }

        // If it's a leaf, check the actual ModelSpheres exactly like your previous loop
        if (node->isLeaf()) {
            std::optional<IntersectionResults> closest = std::nullopt;
            for (const ModelSphere* sphere : node->leafSpheres) {
                std::optional<IntersectionResults> current = rayIntersection(rayStart, rayDirection, *sphere);
                if (current.has_value()) {
                    if (!closest.has_value()) {
                        closest = current;
                    } else {
                        float currentDist = glm::length(current->intersectionPoint - rayStart);
                        float closestDist = glm::length(closest->intersectionPoint - rayStart);
                        if (currentDist < closestDist) {
                            closest = current;
                        }
                    }
                }
            }
            return closest;
        }

        // If it's an internal node, recursively check both children
        auto hitLeft = rayIntersectionBVH(node->left.get(), rayStart, rayDirection);
        auto hitRight = rayIntersectionBVH(node->right.get(), rayStart, rayDirection);

        // Return the closer of the two intersections
        if (hitLeft && hitRight) {
            float distLeft = glm::length(hitLeft->intersectionPoint - rayStart);
            float distRight = glm::length(hitRight->intersectionPoint - rayStart);
            return (distLeft < distRight) ? hitLeft : hitRight;
        }

        return hitLeft ? hitLeft : hitRight;
    }
}