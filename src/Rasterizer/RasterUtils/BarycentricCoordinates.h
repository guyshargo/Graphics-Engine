#pragma once
#include <glm/glm.hpp>
#include <string>

/**
 * @brief Computes and represents barycentric coordinates for a triangle defined by three vertices. 
 *        It provides methods to calculate the barycentric coordinates of a point relative to 
 *        the triangle, check if a point lies inside the triangle, and interpolate properties 
 *        based on these coordinates.
 */
class BarycentricCoordinates {

    glm::vec3 barycentricCoordinates; // Barycentric coordinates (alpha, beta, gamma)

    glm::vec3 lineOppositeToV1;
    glm::vec3 lineOppositeToV2;
    glm::vec3 lineOppositeToV3;

    float d1;
    float d2;
    float d3;

    // Helper methods
    static glm::vec3 lineFromTwoPoints(const glm::vec3& p1, const glm::vec3& p2);
    static float semiDistanceOfPointFromLine(const glm::vec3& line, const glm::vec3& p);
    static float semiDistanceOfPointFromLine(const glm::vec3& line, float x, float y);

    public:
        /**
         * @brief Constructs a BarycentricCoordinates object for a triangle defined by three vertices. 
         *        Vertices are glm::vec3 but only the first two components (x and y) are used.
         *
         * @param v1 The first vertex of the triangle.
         * @param v2 The second vertex of the triangle.
         * @param v3 The third vertex of the triangle.
         */
        BarycentricCoordinates(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

        /**
         * @brief Computes and updates the barycentric coordinates for a specified point.
         *
         * @param x The x-coordinate of the point.
         * @param y The y-coordinate of the point.
         */
        void calcCoordinatesForPoint(float x, float y);

        /**
         * @brief Checks if the currently computed barycentric coordinates indicate that the point lies
         *        inside the triangle.
         *
         * @return True if the point is inside the triangle; false otherwise.
         */
        bool isPointInsideTriangle() const;

        /**
         * @brief Retrieves the calculated weight of the first triangle corner.
         * 
         * @return A float representing the influence of corner one.
         */
        float getAlpha() const;

        /**
         * @brief Retrieves the calculated weight of the second triangle corner.
         * 
         * @return A float representing the influence of corner two.
         */
        float getBeta() const;

        /**
         * @brief Retrieves the calculated weight of the third triangle corner.
         * 
         * @return A float representing the influence of corner three.
         */
        float getGamma() const;

        /**
         * @brief Retrieves the calculated weight of the third triangle corner.
         * 
         * @return A float representing the influence of corner three.
         */
        glm::vec3 getCoordinates() const;
    
        /**
         * @brief Interpolates a scalar property across the triangle.
         *
         * @param p1_property The property value at the first vertex.
         * @param p2_property The property value at the second vertex.
         * @param p3_property The property value at the third vertex.
         * @return The interpolated property value.
         */
        float interpolate(float p1_property, float p2_property, float p3_property) const;

        /**
         * @brief Interpolates a 2D vector property across the triangle.
         *
         * @param p1_property The property value at the first vertex.
         * @param p2_property The property value at the second vertex.
         * @param p3_property The property value at the third vertex.
         * @return The interpolated property as a glm::vec2.
         */
        glm::vec2 interpolate(const glm::vec2& p1_property, const glm::vec2& p2_property, const glm::vec2& p3_property) const;

        /**
         * @brief Interpolates a 3D vector property across the triangle.
         *
         * @param p1_property The property value at the first vertex.
         * @param p2_property The property value at the second vertex.
         * @param p3_property The property value at the third vertex.
         * @return The interpolated property as a glm::vec3.
         */
        glm::vec3 interpolate(const glm::vec3& p1_property, const glm::vec3& p2_property, const glm::vec3& p3_property) const;
};