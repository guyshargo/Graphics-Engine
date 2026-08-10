#include <cmath>
#include <glm/gtc/constants.hpp> // For math constants
#include <glm/trigonometric.hpp> // For radians() and tan()

#include "RayTracerWorld.h"
#include "Model.h"
#include "IntersectionResults.h"
#include "SphereTexture.h"
#include "YourUtilities.h"
#include "ModelSphere.h"
#include "ModelMaterial.h"
#include "Utilities.h"


RayTracerWorld::RayTracerWorld(int imageWidth, int imageHeight, float fovXdegree) : imageWidth(imageWidth), imageHeight(imageHeight) {}
    
RayTracerWorld::~RayTracerWorld() = default;


void RayTracerWorld::setRenderingParams(int depthOfRayTracing) {
    params.setDepthOfRayTracing(depthOfRayTracing);
}


void RayTracerWorld::setExercise(RayTracingExerciseEnum exercise) {
    params.setRtExercise(exercise);
}


bool RayTracerWorld::load(const std::string& filename) {
    try {
        // Load the model from the specified file
        model = std::make_unique<Model>(filename);
        skyBoxImageSphereTexture = std::make_unique<SphereTexture>(model->skyBoxImageFileName);

        std::vector<const ModelSphere*> spherePointers;
        spherePointers.reserve(model->spheres.size());
        for (const auto& sphere : model->spheres) {
            spherePointers.push_back(&sphere);
        }
        bvhRoot = buildBVH(spherePointers, 0);

        return true;

    } catch (const std::exception& e) {
        // Print the error exactly like the Java implementation
        std::cerr << "Failed to load the model file: " << filename 
                  << ".\nDescription: " << e.what() << std::endl;
        return false;
    }
}


glm::vec3 RayTracerWorld::renderPixel(int x, int y) {

    if (!model || !skyBoxImageSphereTexture) {
        return glm::vec3(0.0f);
    }

    glm::vec3 pixelDirection = calcPixelDirection(x, y, imageWidth, imageHeight, model->fovXdegree);
    // Camera position is (0,0,0) with calculated direction vector
    glm::vec3 pixelColor = rayTracing(glm::vec3(0.0f), pixelDirection, *model, *skyBoxImageSphereTexture, 0);
    return pixelColor;
}


glm::vec3 RayTracerWorld::rayTracing(glm::vec3 incidentRayOrigin, glm::vec3 incidentRayDirection, const Model& model, 
            const SphereTexture& skyBoxImageSphereTexture, int depthLevel) const{

    // stopping condition for recursion - return black, no light
    if (depthLevel == params.getDepthOfRayTracing()) {
        return glm::vec3(0.0f);
    }

    // closest intersection of ray with object
    std::optional<IntersectionResults> intersectionResults = rayIntersectionBVH(bvhRoot.get(), incidentRayOrigin, incidentRayDirection);

    // show skyBoxImageSphereTexture if no intersection detected
    if (intersectionResults == std::nullopt) {
        return skyBoxImageSphereTexture.sampleDirectionFromMiddle(incidentRayDirection);
    }

    const ModelSphere* intersectedSphere = intersectionResults -> intersectedSphere;
    const ModelMaterial& intersectedSphereMaterial = model.materials[intersectedSphere -> materialIndex];
    glm::vec3 intersectionPoint = intersectionResults -> intersectionPoint;
    glm::vec3 intersectionNormal = intersectionResults -> normal;
    bool intersectionFromOutsideOfSphere = intersectionResults -> rayFromOutsideOfSphere;
    const SphereTexture& intersectedSphereTexture = model.skyBoxImageSphereTextures[intersectedSphere -> textureIndex];

    // returnedColor will accumulate all light contributions
    glm::vec3 returnedColor(0.0f);

    // adding base sphere color material to returnedColor
    glm::vec3 color = intersectedSphereMaterial.color;
    float kColor = intersectedSphereMaterial.kColor;
    returnedColor += kColor * color;

    // Shadow
    bool isShadow = false;
    if (params.getRtExercise() >= RayTracingExerciseEnum::EX_6_Shadow) {
        isShadow = isPointInShadow(model.lights[0].location, intersectionPoint, intersectionNormal, model);
    }

    /** 
     * lighting
     **/
    glm::vec3 directLighting(0.0f);

    if (!isShadow) {
        // Kd with texture
        glm::vec3 combinedKd = calcKdCombinedWithTexture(intersectionPoint, intersectedSphere->center, intersectedSphereTexture,
                                                         intersectedSphereMaterial.kd, intersectedSphereMaterial.kTexture);

        // 3 light types calculated to one vector
        directLighting = Utilities::lightingEquation(intersectionPoint, intersectionNormal, model.lights[0].location,
            combinedKd, intersectedSphereMaterial.ks, intersectedSphereMaterial.ka, intersectedSphereMaterial.shininess);
    }
    // if its shadowed - kd and ks are 0
    else {
        directLighting = Utilities::lightingEquation(intersectionPoint, intersectionNormal, model.lights[0].location,
            glm::vec3(0.0f), glm::vec3(0.0f), intersectedSphereMaterial.ka, intersectedSphereMaterial.shininess);
    }

    // adding lighting scaled by kDirect to returnedColor
    returnedColor += intersectedSphereMaterial.kDirect * directLighting;

    // adding reflected light if current pixel has reflection value
    if (intersectedSphereMaterial.kReflection > 0.0f && params.getRtExercise() >= RayTracingExerciseEnum::EX_7_Reflection) {
        glm::vec3 reflectedLight = calcReflectedLight(incidentRayDirection, intersectionPoint, intersectionNormal,
            model, skyBoxImageSphereTexture, depthLevel);

        returnedColor += intersectedSphereMaterial.kReflection * reflectedLight;
    }

    // adding transmitted light if current pixel has transmission value
    if (intersectedSphereMaterial.kTransmission > 0.0f && params.getRtExercise() >= RayTracingExerciseEnum::EX_8_Transparency) {
        glm::vec3 transmittedLight = calcTransmissionLight(incidentRayDirection, intersectionPoint, intersectionNormal,
            intersectionFromOutsideOfSphere, intersectedSphereMaterial.refractiveIndex, model, skyBoxImageSphereTexture, depthLevel);

        returnedColor += intersectedSphereMaterial.kTransmission * transmittedLight;
    }

    return returnedColor;
}



glm::vec3 RayTracerWorld::calcPixelDirection(int x, int y, int imageWidth, int imageHeight, float fovXdegree){
    // X axis
    float xLeft = -glm::tan(glm::radians(fovXdegree) / 2.0f);
    int pixelSpacesX = imageWidth - 1;
    float xDelta = (-xLeft * 2.0f) / static_cast<float>(pixelSpacesX);

    // Y axis
    float fovYdegree = fovXdegree/(static_cast<float>(imageWidth) / static_cast<float>(imageHeight));
    float yBottom = -glm::tan(glm::radians(fovYdegree) / 2.0f);
    int pixelSpacesY = imageHeight - 1;
    float yDelta = (-yBottom * 2.0f) / static_cast<float>(pixelSpacesY);

    // vector from eye to specific pixel in window
    glm::vec3 pixelDirection = glm::vec3(xLeft + xDelta * x, yBottom + yDelta * y, -1.0f);
    return glm::normalize(pixelDirection);
}



std::optional<IntersectionResults> RayTracerWorld::rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, 
    const ModelSphere& sphere) {
    
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



glm::vec3 RayTracerWorld::calcKdCombinedWithTexture(glm::vec3 intersectionPoint, 
                                                glm::vec3 intersectedSphereCenter, 
                                                const SphereTexture& intersectedSphereTexture, 
                                                glm::vec3 intersectedSphereKd, 
                                                float kTexture) {

    // normalized direction vector from center of sphere to intersection point
    glm::vec3 dirFromCenterToIntersection = glm::normalize(intersectionPoint - intersectedSphereCenter);

    // get texture color of point according to the direction from center to intersection point
    glm::vec3 textureColor = intersectedSphereTexture.sampleDirectionFromMiddle(dirFromCenterToIntersection);

    // combine texture color with diffuse reflection coefficient
    return kTexture * textureColor + (1.0f - kTexture) * intersectedSphereKd;
}



bool RayTracerWorld::isPointInShadow(glm::vec3 lightLocation, glm::vec3 point, glm::vec3 pointNormal, const Model& model) const {

    // Vector from point to light source to check if is being intersected along the way
    glm::vec3 shadowRay = glm::normalize(lightLocation - point);

    // adjusting point on model by 0.05 in normal direction
    glm::vec3 outsideOfObjectPoint = point + 0.05f * pointNormal;

    // check collision with other spheres along shadowRay
    std::optional<IntersectionResults> isCollision = rayIntersectionBVH(bvhRoot.get(), outsideOfObjectPoint, shadowRay);

    // Return true if the point is in shadow, false otherwise
    return isCollision.has_value();

}



glm::vec3 RayTracerWorld::calcReflectedLight(glm::vec3 incidentRayDirection, glm::vec3 intersectionPoint, glm::vec3 intersectionNormal,
    const Model& model, const SphereTexture& skyBoxImageSphereTexture, int depthLevel) const {
    
    glm::vec3 normal = glm::normalize(intersectionNormal);

    // negate eye direction (object to camera), to match direction with reflectionRay for calculation
    glm::vec3 v = - glm::normalize(incidentRayDirection);
    float NdotV = glm::dot(normal, v);

    // Calculate reflection vector R, where the light ray bounces to
    glm::vec3 reflectionRay = 2.0f * NdotV * normal - v;

    // calculating point outside of object by 0.05 in normal direction
    // prevents reflected ray from hitting the same object
    glm::vec3 outsideOfObjectPoint = intersectionPoint + 0.05f * normal;

    return rayTracing(outsideOfObjectPoint, reflectionRay, model, skyBoxImageSphereTexture, depthLevel + 1);
}



glm::vec3 RayTracerWorld::calcTransmissionLight(glm::vec3 incidentRayDirection, 
                                    glm::vec3 intersectionPoint, 
                                    glm::vec3 intersectionNormal,
                                    bool intersectionFromOutsideOfSphere, 
                                    float refractiveIndexIntersectedSphere, 
                                    const Model& model, 
                                    const SphereTexture& skyBoxImageSphereTexture, 
                                    int depthLevel) const {
    
    // calculate new direction of ray after bend through material
    glm::vec3 transmittedRay = YourUtilities::calcTransmissionRay(incidentRayDirection, intersectionNormal, refractiveIndexIntersectedSphere, intersectionFromOutsideOfSphere);
    
    // if resulted vector is very small - its reflected light so we send ray to be calculated as such
    if (glm::length(transmittedRay) < 0.0001f) {
        return calcReflectedLight(incidentRayDirection, intersectionPoint, intersectionNormal, model, skyBoxImageSphereTexture, depthLevel);
    }

    glm::vec3 normal = glm::normalize(intersectionNormal);

    // calculating point inside of object by 0.05 in normal direction
    // prevent refracted ray from immediately intersecting the surface it just entered
    glm::vec3 insideOfObjectPoint = intersectionPoint - 0.05f * normal;

    return rayTracing(insideOfObjectPoint, transmittedRay, model, skyBoxImageSphereTexture, depthLevel + 1);
}



BoundingSphere RayTracerWorld::computeBoundingSphere(const std::vector<const ModelSphere*>& spheres) const {
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



std::unique_ptr<BVHNode> RayTracerWorld::buildBVH(std::vector<const ModelSphere*> spheres, int depth) {
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



bool RayTracerWorld::intersectBoundingSphere(const glm::vec3& rayStart, const glm::vec3& rayDirection, const BoundingSphere& bounds) const {
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



std::optional<IntersectionResults> RayTracerWorld::rayIntersectionBVH(const BVHNode* node, const glm::vec3& rayStart, const glm::vec3& rayDirection) const {
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