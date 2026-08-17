#pragma once
#include <glm/glm.hpp>
#include "./Scene/ModelSphere.h"

/**
 * @brief Stores the exact details of a collision between a light ray and a 3D object, 
 *        providing the necessary location and surface direction to calculate lighting, 
 *        shadows, and reflections.
 */
struct IntersectionResults {

    /**
     * @brief Creates a record of a ray collision to be used for calculating lighting, shadows, and reflections.
     * 
     * @param intersected True if the ray successfully hit an object.
     * @param intersectionPoint The exact 3D coordinates where the light ray hit the object's surface.
     * @param normal The direction the surface is facing at the exact point of impact.
     * @param rayFromOutsideOfSphere True if the light ray hit the exterior shell, false if it is bouncing inside a transparent object.
     * @param intersectedSphere The specific 3D object that was hit by the ray.
     */
    IntersectionResults(bool intersected, const glm::vec3& intersectionPoint, const glm::vec3& normal, bool rayFromOutsideOfSphere, 
                        const ModelSphere* intersectedSphere)                               
                        :                            
                        intersectionPoint(intersectionPoint), normal(normal), rayFromOutsideOfSphere(rayFromOutsideOfSphere), 
                        intersectedSphere(intersectedSphere) {}

    // The exact 3D coordinates where the light ray hit the object's surface
    glm::vec3 intersectionPoint;
    
    // The direction the surface is facing at the exact point of impact
    glm::vec3 normal;
    
    // Flag indicating if the ray originated from outside of the sphere
    bool rayFromOutsideOfSphere;
    
    // The ModelSphere object that was intersected by the ray
    const ModelSphere* intersectedSphere;

};