#include "BarycentricCoordinates.h"

// Constructor
BarycentricCoordinates::BarycentricCoordinates(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {

    // Generate the mathematical line equations for the three edges of the triangle
    lineOppositeToV1 = lineFromTwoPoints(v2, v3);
    lineOppositeToV2 = lineFromTwoPoints(v1, v3);
    lineOppositeToV3 = lineFromTwoPoints(v1, v2);

    // Pre-calculate the inverse maximum distances from each corner to its opposite edge
    // This allows the engine to use fast multiplication instead of slow division per-pixel later
    d1 = 1.0f / semiDistanceOfPointFromLine(lineOppositeToV1, v1);
    d2 = 1.0f / semiDistanceOfPointFromLine(lineOppositeToV2, v2);
    d3 = 1.0f / semiDistanceOfPointFromLine(lineOppositeToV3, v3);
}

// Accessors
float BarycentricCoordinates::getAlpha() const { return barycentricCoordinates.x; }
float BarycentricCoordinates::getBeta() const { return barycentricCoordinates.y; }
float BarycentricCoordinates::getGamma() const { return barycentricCoordinates.z; }
glm::vec3 BarycentricCoordinates::getCoordinates() const { return barycentricCoordinates; }


void BarycentricCoordinates::calcCoordinatesForPoint(float x, float y) {

    // Calculate the exact ratio of how close the current pixel is to each corner
    float barycentricAlpha = semiDistanceOfPointFromLine(lineOppositeToV1, x, y) * d1;
    float barycentricBeta = semiDistanceOfPointFromLine(lineOppositeToV2, x, y) * d2;
    float barycentricGamma = semiDistanceOfPointFromLine(lineOppositeToV3, x, y) * d3;

    barycentricCoordinates = glm::vec3(barycentricAlpha, barycentricBeta, barycentricGamma);
}

bool BarycentricCoordinates::isPointInsideTriangle() const {
    // If any weight is negative, the pixel has crossed outside the lines of the triangle
    // If any weight is above 1.0, the pixel is too far past a specific corner
    return ((barycentricCoordinates.x >= 0) && (barycentricCoordinates.x <= 1)
            && (barycentricCoordinates.y >=0) && (barycentricCoordinates.y <= 1)
            && (barycentricCoordinates.z >= 0) && (barycentricCoordinates.z <=1));
}

float BarycentricCoordinates::interpolate(float p1_property, float p2_property, float p3_property) const {
    return barycentricCoordinates.x * p1_property 
            + barycentricCoordinates.y * p2_property 
            + barycentricCoordinates.z * p3_property;
}

glm::vec2 BarycentricCoordinates::interpolate(const glm::vec2& p1_property, const glm::vec2& p2_property, const glm::vec2& p3_property) const {
    
    // Multiply each corner's X and Y coordinates by its respective weight and sum them together
    float x = p1_property.x * barycentricCoordinates.x
            + p2_property.x * barycentricCoordinates.y
            + p3_property.x * barycentricCoordinates.z;

    float y = p1_property.y * barycentricCoordinates.x
            + p2_property.y * barycentricCoordinates.y
            + p3_property.y * barycentricCoordinates.z;

    return glm::vec2(x, y);
}

glm::vec3 BarycentricCoordinates::interpolate(const glm::vec3& p1_property, const glm::vec3& p2_property, const glm::vec3& p3_property) const {
    
    // Multiply each corner's X, Y, and Z coordinates by its respective weight and sum them together
    float x = p1_property.x * barycentricCoordinates.x
            + p2_property.x * barycentricCoordinates.y
            + p3_property.x * barycentricCoordinates.z;

    float y = p1_property.y * barycentricCoordinates.x
            + p2_property.y * barycentricCoordinates.y
            + p3_property.y * barycentricCoordinates.z;

    float z = p1_property.z * barycentricCoordinates.x
            + p2_property.z * barycentricCoordinates.y
            + p3_property.z * barycentricCoordinates.z;

    return glm::vec3(x, y, z);
}

// Generates the A, B, and C coefficients for the standard line equation (Ax + By + C = 0)
glm::vec3 BarycentricCoordinates::lineFromTwoPoints(const glm::vec3& p1, const glm::vec3& p2) {
    return glm::vec3 (p2.y - p1.y, -(p2.x - p1.x), (p1.y * p2.x) - (p1.x * p2.y));
}

// Calculates the raw, unscaled distance from a point to a specific line
float BarycentricCoordinates::semiDistanceOfPointFromLine(const glm::vec3& line, const glm::vec3& p) {
    return line.x * p.x + line.y * p.y + line.z;
}

float BarycentricCoordinates::semiDistanceOfPointFromLine(const glm::vec3& line, float x, float y) {
    return line.x * x + line.y * y + line.z;
}