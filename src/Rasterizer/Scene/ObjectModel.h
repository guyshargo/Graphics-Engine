#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "./Data/PipelineData.h"
#include "OBJLoader.h"
#include "./Textures/Texture2D.h"

class RasterizerWorld;

/**
 * @brief Manages the geometry of a single 3D model, pushing its triangles through the 
 *        rendering pipeline: moving them into camera space, cutting off parts outside the 
 *        screen, and calculating the final colored pixels.
 */
class ObjectModel {
public:

    /**
     * @brief Initializes the model with references to the world environment and screen size.
     * 
     * @param rasterizerWorld Pointer to the main engine coordinator.
     * @param imageWidth The horizontal pixel count of the screen.
     * @param imageHeight The vertical pixel count of the screen.
     */
    ObjectModel(RasterizerWorld* rasterizerWorld, int imageWidth, int imageHeight);

    /**
     * @brief Sets all 3D transformation matrices to their default neutral state.
     */
    void initTransformations();

    /**
     * @brief Updates the transformation matrix handling the physical position of the model.
     * 
     * @param modelM The matrix that scales, rotates, and places the 3D model into the global world.
     */
    void setModelM(const glm::mat4& modelM);

    /**
     * @brief Updates the transformation matrix handling the camera's location and angle.
     * 
     * @param lookatM The matrix that shifts the 3D world to match the camera's perspective.
     */
    void setLookatM(const glm::mat4& lookatM);

    /**
     * @brief Updates the transformation matrix handling the lens distortion.
     * 
     * @param projectionM The matrix that flattens the 3D coordinates based on field of view and distance.
     */
    void setProjectionM(const glm::mat4& projectionM);

    /**
     * @brief Updates the transformation matrix handling the final screen stretch.
     * 
     * @param viewportM The matrix that stretches the flattened coordinates to fit the actual pixel dimensions of the screen.
     */
    void setViewportM(const glm::mat4& viewportM);

    /**
     * @brief Retrieves the physical dimensions of the 3D model.
     * 
     * @return The full width, height, and depth of the 3D model.
     */
    glm::vec3 getBoundingBoxDimensions() const;

    /**
     * @brief Retrieves the exact center coordinate of the 3D model.
     * 
     * @return The exact middle 3D coordinate of the model.
     */
    glm::vec3 getBoundingBoxCenter() const;

    /**
     * @brief Directs the model to parse an .obj file from the hard drive and extract its geometry.
     * 
     * @param fileName The system path to the 3D model file.
     * @return True if the file successfully loaded, false otherwise.
     */
    bool load(const std::string& fileName);

    /**
     * @brief Checks if this specific model has a loaded image texture.
     * 
     * @return True if the model has a valid image texture assigned to it, false otherwise.
     */
    bool objectHasTexture() const;

    /**
     * @brief Defines the expected function signature for plotting a single RGB color to a specific screen coordinate.
     * 
     * @param x The horizontal screen pixel coordinate.
     * @param y The vertical screen pixel coordinate.
     * @param color The final computed RGB color for the pixel.
     */
    using PlotPixelCallback = std::function<void(int x, int y, const glm::vec3& color)>;

    /**
     * @brief Initiates the graphics pipeline sequence for every triangle in the model.
     * 
     * @param plotPixel The function executed to draw a calculated color onto a specific screen coordinate.
     */
    void render(const PlotPixelCallback& plotPixel);

private:
    RasterizerWorld* rasterizerWorld;
    int imageWidth;
    int imageHeight;

    std::vector<VertexData> verticesData;
    std::vector<TriangleFace> faces;

    std::unique_ptr<Texture2D> texture;

    glm::mat4 modelM{1.0f};
    glm::mat4 lookatM{1.0f};
    glm::mat4 projectionM{1.0f};
    glm::mat4 viewportM{1.0f};
    
    glm::vec3 boundingBoxDimensions;
    glm::vec3 boundingBoxCenter;
    glm::vec3 lightPositionEyeCoordinates;

    /**
     * @brief Moves a single 3D point from its original model shape into the global world space, 
     *        then relative to the camera position.
     * 
     * @param plotPixel The rendering function to pass along the pipeline.
     * @param vertex The specific 3D coordinate package to transform into camera space.
     */
    void vertexProcessing(const PlotPixelCallback& plotPixel, VertexData& vertex);

    /**
     * @brief Scans the rectangular screen area around a 3D triangle, determining exactly which 
     *        screen pixels fall inside the boundaries and should be colored.
     * 
     * @param plotPixel The rendering function.
     * @param vertex1 The first transformed corner of the triangle.
     * @param vertex2 The second transformed corner of the triangle.
     * @param vertex3 The third transformed corner of the triangle.
     * @param faceColor The base solid color of the triangle.
     */
    void rasterization(const PlotPixelCallback& plotPixel, const VertexData& vertex1, const VertexData& vertex2, const VertexData& vertex3, const glm::vec3& faceColor);
    
    /**
     * @brief Applies the final perspective division to flatten the 3D coordinate onto the 2D screen.
     * 
     * @param plotPixel The rendering function.
     * @param vertex The coordinate package to flatten into 2D screen space.
     */
    void finalizeVertex(const PlotPixelCallback& plotPixel, VertexData& vertex);

    /**
     * @brief Calculates a brand new coordinate and its associated color data exactly between two existing points.
     * 
     * @param v1 The starting coordinate package.
     * @param v2 The ending coordinate package.
     * @param interpolationWeight The exact decimal fraction between the two points to calculate.
     * @return A newly calculated vertex containing the averaged data.
     */
    static VertexData interpolateVertex(const VertexData& v1, const VertexData& v2, float interpolationWeight);

    /**
     * @brief Slices a triangle that crosses the edge of the screen, discarding the off-screen piece 
     *        and creating new triangles out of the remaining visible piece.
     * 
     * @param planePoint A 3D point on the cutting boundary.
     * @param planeNormal The direction the cutting boundary faces.
     * @param inputTriangle The original triangle that crosses the boundary.
     * @param outTriangles The array that will be populated with the newly cut triangles.
     * @return The number of new triangles successfully generated (0, 1, or 2).
     */
    int clipTriangleAgainstPlane(glm::vec3 planePoint, glm::vec3 planeNormal, 
                             const std::array<VertexData, 3>& inputTriangle, 
                             std::array<std::array<VertexData, 3>, 2>& outTriangles);
               
    /**
     * @brief Calculates the final visible color of a single pixel on a surface, applying lighting math or image textures.
     * 
     * @param fragmentData The package containing the exact lighting and texture data for a single screen pixel.
     * @return The final computed RGB color for that pixel.
     */
    glm::vec3 fragmentProcessing(const FragmentData& fragmentData);

    /**
     * @brief Identifies the smallest possible 2D screen rectangle that completely contains a given triangle.
     * 
     * @param p1 The first 2D screen coordinate of the triangle.
     * @param p2 The second 2D screen coordinate of the triangle.
     * @param p3 The third 2D screen coordinate of the triangle.
     * @param imageWidth The total width of the screen to prevent the box from exceeding the visible area.
     * @param imageHeight The total height of the screen to prevent the box from exceeding the visible area.
     * @return The minimum and maximum X and Y coordinates forming a flat rectangle around the triangle.
     */
    static glm::ivec4 calcBoundingBox(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, int imageWidth, int imageHeight);
};