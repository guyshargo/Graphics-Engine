#include <cmath>
#include <glm/gtc/constants.hpp> // For math constants
#include <glm/trigonometric.hpp> // For radians() and tan()

#include "WorldModel.h"
#include "Model.h"
#include "IntersectionResults.h"
#include "SphereTexture.h"
#include "YourUtilities.h"
#include "ModelSphere.h"
#include "ModelMaterial.h"

// Constructor implementation initializing variables
WorldModel::WorldModel(int imageWidth, int imageHeight, float fovXdegree)
    : imageWidth(imageWidth), imageHeight(imageHeight) {
}
WorldModel::~WorldModel() = default; // Destructor implementation to clean up resources

void WorldModel::setRenderingParams(int depthOfRayTracing) {
    params.setDepthOfRayTracing(depthOfRayTracing);
}

void WorldModel::setExercise(ExerciseEnum exercise) {
    params.setExercise(exercise);
}

bool WorldModel::load(const std::string& filename) {
    try {
        // Load the model from the specified file
        model = std::make_unique<Model>(filename);
        skyBoxImageSphereTexture = std::make_unique<SphereTexture>(model->skyBoxImageFileName);
        return true;

    } catch (const std::exception& e) {
        // Print the error exactly like the Java implementation
        std::cerr << "Failed to load the model file: " << filename 
                  << ".\nDescription: " << e.what() << std::endl;
        return false;
    }
}

glm::vec3 WorldModel::renderPixel(int x, int y) {

    if (!model || !skyBoxImageSphereTexture) {
        return glm::vec3(0.0f);
    }

    glm::vec3 pixelDirection = calcPixelDirection(x, y, imageWidth, imageHeight, model->fovXdegree);
    // Camera position is (0,0,0) with calculated direction vector
    glm::vec3 pixelColor = rayTracing(glm::vec3(0.0f), pixelDirection, *model, *skyBoxImageSphereTexture, 0);
    return pixelColor;
}

glm::vec3 WorldModel::rayTracing(glm::vec3 incidentRayOrigin, glm::vec3 incidentRayDirection, const Model& model, 
            const SphereTexture& skyBoxImageSphereTexture, int depthLevel) const{

    // stopping condition for recursion - return black, no light
    if (depthLevel == params.getDepthOfRayTracing()) {
        return glm::vec3(0.0f);
    }

    // closest intersection of ray with object
    std::optional<IntersectionResults> intersectionResults = rayIntersection(incidentRayOrigin, incidentRayDirection, model.spheres);

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
    bool isShadow = isPointInShadow(model.lights[0].location, intersectionPoint, intersectionNormal, model);

    /** 
     * lighting
     **/
    glm::vec3 directLighting(0.0f);

    if (!isShadow) {
        // Kd with texture
        glm::vec3 combinedKd = calcKdCombinedWithTexture(intersectionPoint, intersectedSphere->center, intersectedSphereTexture,
                                                         intersectedSphereMaterial.kd, intersectedSphereMaterial.kTexture);

        // 3 light types calculated to one vector
        directLighting = lightingEquation(intersectionPoint, intersectionNormal, model.lights[0].location,
            combinedKd, intersectedSphereMaterial.ks, intersectedSphereMaterial.ka, intersectedSphereMaterial.shininess);
    }
    // if its shadowed - kd and ks are 0
    else {
        directLighting = lightingEquation(intersectionPoint, intersectionNormal, model.lights[0].location,
            glm::vec3(0.0f), glm::vec3(0.0f), intersectedSphereMaterial.ka, intersectedSphereMaterial.shininess);
    }

    // adding lighting scaled by kDirect to returnedColor
    returnedColor += intersectedSphereMaterial.kDirect * directLighting;

    // adding reflected light if current pixel has reflection value
    if (intersectedSphereMaterial.kReflection > 0.0f) {
        glm::vec3 reflectedLight = calcReflectedLight(incidentRayDirection, intersectionPoint, intersectionNormal,
            model, skyBoxImageSphereTexture, depthLevel);

        returnedColor += intersectedSphereMaterial.kReflection * reflectedLight;
    }

    // adding transmitted light if current pixel has transmission value
    if (intersectedSphereMaterial.kTransmission > 0.0f) {
        glm::vec3 transmittedLight = calcTransmissionLight(incidentRayDirection, intersectionPoint, intersectionNormal,
            intersectionFromOutsideOfSphere, intersectedSphereMaterial.refractiveIndex, model, skyBoxImageSphereTexture, depthLevel);

        returnedColor += intersectedSphereMaterial.kTransmission * transmittedLight;
    }

    return returnedColor;
}

glm::vec3 WorldModel::calcPixelDirection(int x, int y, int imageWidth, int imageHeight, float fovXdegree){
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

glm::vec3 WorldModel::lightingEquation(glm::vec3 point, glm::vec3 pointNormal, glm::vec3 lightPosition, glm::vec3 kd, glm::vec3 ks,
                                        glm::vec3 ka, float shininess) {
    glm::vec3 returnedColor(0.0f);

    glm::vec3 normal = glm::normalize(pointNormal);
    glm::vec3 lightDir = glm::normalize(lightPosition - point);

    // cos of angle between light direction and normal
    float NdotL = glm::dot(normal, lightDir);

    /*******
    Diffusive Lighting: max between 0 (back of the surface so no light) and NdotL
    ********/
    float diffusiveLight = glm::max(NdotL, 0.0f);
    returnedColor += diffusiveLight * kd;

    /*******
    Specular Lighting
    ********/
    // if light is in front of object calculate specular light
    if (NdotL >= 0) {
        // Calculate reflection vector R, where the light ray bounces to
        glm::vec3 specularR = glm::normalize(2.0f * NdotL * normal - lightDir);

        // Direction vector of eye - from intersection point to camera
        glm::vec3 specularV = glm::normalize(-point);

        // if returned ray and eye direction are opposite to each other take max
        float RdotV = glm::max(glm::dot(specularR, specularV), 0.0f);

        // Adding shininess factor - how concentrated is the shine
        float specularLight = glm::pow(RdotV, shininess);
        returnedColor += specularLight * ks;
    }

    /*******
    Ambient Lighting: constant light that is always present
    ********/
    returnedColor += ka;

    return returnedColor;
}

    std::optional<IntersectionResults> WorldModel::rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, 
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

    std::optional<IntersectionResults> WorldModel::rayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, 
        const std::vector<ModelSphere>& spheres){

        //IntersectionResults object which will resemble the closest sphere which was intersected
        std::optional<IntersectionResults> closestSphereIntersection = std::nullopt;

        for (const ModelSphere& sphere : spheres) {
            std::optional<IntersectionResults> currentIntersection = rayIntersection(rayStart, rayDirection, sphere);

            if (currentIntersection.has_value()) {
                // if first intersection, set closest intersection to the first intersection
                if (closestSphereIntersection == std::nullopt) {
                    closestSphereIntersection = currentIntersection;
                }
                else {
                    // calculate current distance of intersections
                    float currentDistance = glm::length(currentIntersection->intersectionPoint - rayStart);
                    float closestDistance = glm::length(closestSphereIntersection->intersectionPoint - rayStart);

                    // check minimal distance and update closer sphere
                    if (currentDistance < closestDistance) {
                        closestSphereIntersection = currentIntersection;
                    }
                }
            }
        }
        // return the closest sphere each time, or null
        return closestSphereIntersection;
    }

    glm::vec3 WorldModel::calcKdCombinedWithTexture(glm::vec3 intersectionPoint, 
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


    bool WorldModel::isPointInShadow(glm::vec3 lightLocation, glm::vec3 point, glm::vec3 pointNormal, const Model& model) {

        // Vector from point to light source to check if is being intersected along the way
        glm::vec3 shadowRay = glm::normalize(lightLocation - point);

        // adjusting point on model by 0.05 in normal direction
        glm::vec3 outsideOfObjectPoint = point + 0.05f * pointNormal;

        // check collision with other spheres along shadowRay
        std::optional<IntersectionResults> isCollision = rayIntersection(outsideOfObjectPoint, shadowRay, model.spheres);

        // Return true if the point is in shadow, false otherwise
        return isCollision.has_value();

    }

    glm::vec3 WorldModel::calcReflectedLight(glm::vec3 incidentRayDirection, glm::vec3 intersectionPoint, glm::vec3 intersectionNormal,
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

    glm::vec3 WorldModel::calcTransmissionLight(glm::vec3 incidentRayDirection, 
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
