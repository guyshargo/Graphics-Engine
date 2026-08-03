#pragma once
#include <glm/glm.hpp>
#include "ModelSphere.h"

struct IntersectionResults {
    // The point of intersection on the ray
    glm::vec3 intersectionPoint;
    
    // The normal vector at the point of intersection
    glm::vec3 normal;
    
    // Flag indicating if the ray originated from outside of the sphere
    bool rayFromOutsideOfSphere;
    
    // Pointer to the ModelSphere object that was intersected
    const ModelSphere* intersectedSphere;

    // Constructor
    IntersectionResults(bool intersected, const glm::vec3& intersectionPoint, const glm::vec3& normal, bool rayFromOutsideOfSphere, 
                        const ModelSphere* intersectedSphere)
                                
                        : 
                                
            intersectionPoint(intersectionPoint), normal(normal), rayFromOutsideOfSphere(rayFromOutsideOfSphere), 
            intersectedSphere(intersectedSphere) {}
};