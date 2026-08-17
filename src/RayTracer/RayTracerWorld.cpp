#include <cmath>
#include <random>
#include <iostream>
#include <glm/gtc/constants.hpp> // For math constants
#include <glm/trigonometric.hpp> // For radians() and tan()

#include "RayTracerWorld.h"
#include "./Scene/Model.h"
#include "./Data/IntersectionResults.h"
#include "./Textures/SphereTexture.h"
#include "./RayUtils/OpticsUtils.h"
#include "./Scene/ModelSphere.h"
#include "./Scene/ModelMaterial.h"
#include "LightingUtils.h"
#include "./Acceleration/BVH.h"

RayTracerWorld::RayTracerWorld(int imageWidth, int imageHeight, float fovXdegree) : imageWidth(imageWidth), imageHeight(imageHeight) {}
    
RayTracerWorld::~RayTracerWorld() = default;


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
        bvhRoot = BVH::buildBVH(spherePointers, 0);

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
        return glm::vec3(0.0f); // Return black when the scene is not loaded
    }

    thread_local std::random_device rd;
    thread_local std::mt19937 generator(rd());

    glm::vec3 cameraOrigin(0.0f);
    // Accumulate the color returned by each sampled ray
    glm::vec3 accumulatedPixelColor(0.0f);
    
    // Generate offset from the pixel center for each sample
    std::uniform_real_distribution<float> subPixels(DefaultParams::MIN_SUBPIXELS_RANGE, DefaultParams::MAX_SUBPIXELS_RANGE);
    std::uniform_real_distribution<float> lensSample(0.0f, 1.0f);
    
    int antialiasingSamples = params.getAntialiasingSamples();

    for (int i = 0; i < antialiasingSamples; i++) {

        float subX = x + subPixels(generator);
        float subY = y + subPixels(generator);

        // Generate and trace a ray through a sample's subpixel position
        glm::vec3 pixelDirection = calcPixelDirection(subX, subY, imageWidth, imageHeight, model->fovXdegree);

        // Finding focal point where the main ray intersects the focus plane
        glm::vec3 focusPlanePoint = cameraOrigin + (pixelDirection * params.getFocalDistance());

        // Lens Sampling
        float randomRadius = params.getAperatureRadius() * std::sqrt(lensSample(generator));
        float theta = 2.0f * glm::pi<float>() * lensSample(generator);

        float lensX = randomRadius * std::cos(theta);
        float lensY = randomRadius * std::sin(theta);

        // Calculate the specific origin for this sample
        glm::vec3 sampleOrigin = glm::vec3(lensX, lensY, 0.0f);
        glm::vec3 sampleDirection = glm::normalize(focusPlanePoint - sampleOrigin);

        // Camera original position is (0,0,0) with calculated direction vector of sample
        glm::vec3 pixelColor = rayTracing(sampleOrigin, sampleDirection, *model, *skyBoxImageSphereTexture, 0);

        accumulatedPixelColor += pixelColor;
    }
    
    // Average the samples for the final antialiased pixel color
    return accumulatedPixelColor /= antialiasingSamples;
}


glm::vec3 RayTracerWorld::rayTracing(glm::vec3 incidentRayOrigin, glm::vec3 incidentRayDirection, const Model& model, 
            const SphereTexture& skyBoxImageSphereTexture, int depthLevel) const{

    // stopping condition for recursion - return black, no light
    if (depthLevel == params.getDepthOfRayTracing()) {
        return glm::vec3(0.0f);
    }

    // closest intersection of ray with object
    std::optional<IntersectionResults> intersectionResults = BVH::rayIntersectionBVH(bvhRoot.get(), incidentRayOrigin, incidentRayDirection);

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

    /** 
     * lighting
     **/

    // Kd with texture
    glm::vec3 combinedKd = calcKdCombinedWithTexture(intersectionPoint, intersectedSphere->center, intersectedSphereTexture,
                                                        intersectedSphereMaterial.kd, intersectedSphereMaterial.kTexture);

    // Calculate diffuse, specular, and ambient lighting without shadows
    glm::vec3 fullLighting = LightingUtils::lightingEquation(intersectionPoint, intersectionNormal, model.lights[0].location,
        combinedKd, intersectedSphereMaterial.ks, intersectedSphereMaterial.ka, intersectedSphereMaterial.shininess);

    // Calculate only the ambient light so shadowed surfaces retain it
    glm::vec3 ambientLighting = LightingUtils::lightingEquation(intersectionPoint, intersectionNormal, model.lights[0].location,
                                glm::vec3(0.0f), glm::vec3(0.0f), intersectedSphereMaterial.ka, intersectedSphereMaterial.shininess);

    // Calculate the fraction of sampled light rays blocked by geometry
    float shadowFactor = isPointInShadow(model.lights[0].location, intersectionPoint, intersectionNormal, model);

    // Linear interpolation (mix) between full lighting and ambient-only lighting based on shadow factor
    glm::vec3 directLighting = glm::mix(fullLighting, ambientLighting, shadowFactor);

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



glm::vec3 RayTracerWorld::calcPixelDirection(float x, float y, int imageWidth, int imageHeight, float fovXdegree){
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



float RayTracerWorld::isPointInShadow(glm::vec3 lightLocation, glm::vec3 point, glm::vec3 pointNormal, const Model& model) const {

    int blockedRays = 0;
    float lightRadius = model.lights[0].radius;
    int shadowSamples = params.getSoftShadowSamples();

    thread_local std::random_device rd;
    thread_local std::mt19937 generator(rd());
    // Generate offset from the pixel center for each sample
    std::uniform_real_distribution<float> subPixels(0.0f, 1.0f);

    // --- Constructing 3D orientation of light blocking disk ---

    // W: Direction from light to the intersection point
    glm::vec3 forwardW_axis = glm::normalize(point - lightLocation);

    // Choose an arbitrary up vector
    // If W is too close to the Y-axis, use the X-axis instead to prevent a zero cross product
    glm::vec3 arbitraryUp = (std::abs(forwardW_axis.y) > 0.999f) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

    // U and V: The perpendicular axes representing the disk's 2D plane
    glm::vec3 rightU_axis = glm::normalize(glm::cross(arbitraryUp, forwardW_axis));
    glm::vec3 upV_axis = glm::cross(forwardW_axis, rightU_axis);

    for (int i = 0; i < shadowSamples; i++) {

        float r = lightRadius * std::sqrt(subPixels(generator));
        float theta = 2.0f * glm::pi<float>() * subPixels(generator);

        float rX = r * std::cos(theta);
        float rY = r * std::sin(theta);

        // Calculate final 3D point on the disk
        glm::vec3 shadowRayOrigin = rX * rightU_axis + rY * upV_axis + lightLocation;

        // The shadow ray you will cast to test for occlusion
        glm::vec3 shadowRayDirection = glm::normalize(shadowRayOrigin - point);

        // adjusting point on model by 0.05 in normal direction
        glm::vec3 outsideOfObjectPoint = point + 0.05f * pointNormal;

        // check collision with other spheres along shadowRay
        std::optional<IntersectionResults> isCollision = BVH::rayIntersectionBVH(bvhRoot.get(), outsideOfObjectPoint, shadowRayDirection);

        if (isCollision.has_value()) {
            // Calculate distances to ensure the blocking object is actually between the surface and the light
            float distanceToLight = glm::length(shadowRayOrigin - outsideOfObjectPoint);
            float distanceToHit = glm::length(isCollision->intersectionPoint - outsideOfObjectPoint);
            
            // Only count the ray as blocked if the object is closer than the light
            if (distanceToHit < distanceToLight) {
                blockedRays++;
            }
        }
    }

    float shadowFactor = static_cast<float>(blockedRays) / static_cast<float>(shadowSamples);
    return shadowFactor;

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
    glm::vec3 transmittedRay = OpticsUtils::calcTransmissionRay(incidentRayDirection, intersectionNormal, refractiveIndexIntersectedSphere, intersectionFromOutsideOfSphere);
    
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

void RayTracerWorld::setDepthOfRayTracing(int depthOfRayTracing) {
    params.setDepthOfRayTracing(depthOfRayTracing);
}

void RayTracerWorld::setAntialiasingSamples(int antialiasingSamples) {
    params.setAntialiasingSamples(antialiasingSamples);
}

void RayTracerWorld::setSoftShadowSamples(int softShadowSamples) {
    params.setSoftShadowSamples(softShadowSamples);
}

void RayTracerWorld::setExercise(RayTracingExerciseEnum exercise) {
    params.setRtExercise(static_cast<int>(exercise));
}

void RayTracerWorld::setAperatureRadius(float aperatureRadius) {
    params.setAperatureRadius(aperatureRadius);
}

void RayTracerWorld::setFocalDistance(float focalDistance) {
    params.setFocalDistance(focalDistance);
}