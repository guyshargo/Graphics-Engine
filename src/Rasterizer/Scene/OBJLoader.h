#pragma once

#include <string>
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include <SDL.h>
#include "./Data/PipelineData.h"

/**
 * @brief Reads standard .obj 3D model files from the hard drive, extracting the raw 
 *        coordinate points, triangles, and surface directions so the engine can draw the shape.
 */
class OBJLoader {
public:
    /**
     * @brief Configures the loader to expect either round or flat models.
     * 
     * @param useSphericalMapping True to wrap the image around the object like a globe, false to project it flatly.
     */
    OBJLoader(bool useSphericalMapping = true);
    
    /**
     * @brief Reads the geometry file from the hard drive and extracts all points and triangles into memory.
     * 
     * @param filePath The system path to the .obj file.
     */
    void loadOBJ(const std::string& filePath);

    /**
     * @brief Retrieves the calculated physical dimensions of the loaded 3D model.
     * 
     * @return The full width, height, and depth of the loaded geometry.
     */
    glm::vec3 getBoundingBoxDimensions() const;

    /**
     * @brief Retrieves the calculated exact center of the loaded 3D model.
     * 
     * @return The exact middle 3D coordinate of the loaded geometry.
     */
    glm::vec3 getBoundingBoxCenter() const;

    /**
     * @brief Retrieves the extracted points that make up the model.
     * 
     * @return The complete list of 3D points making up the model.
     */
    const std::vector<VertexData>& getVertices() const { return vertices; }

    /**
     * @brief Retrieves the extracted surfaces that connect the points of the model.
     * 
     * @return The complete list of triangles connecting the points.
     */
    const std::vector<TriangleFace>& getFaces() const { return faces; }

    /**
     * @brief Retrieves the path to the texture assigned to this model.
     * 
     * @return The system path to the image file associated with the model.
     */
    const std::string& getTextureFilePath() const { return textureFilePath; }

private:
    /**
     * @brief Averages the flat surface directions of surrounding triangles to artificially 
     *        round out the lighting on blocky, low-polygon 3D models.
     */
    void calculateNormalsSmoothShading();

    /**
     * @brief Generates 2D texture coordinates by mathematically wrapping the 3D points around 
     *        a central axis, commonly used for globes and skyboxes.
     */
    void calculateSphericalMapping();

    /**
     * @brief Generates 2D texture coordinates by projecting the 3D points straight down 
     *        onto a flat surface.
     */
    void calculatePlanarMapping();

    bool useSphericalMapping;
    std::vector<VertexData> vertices;
    std::vector<TriangleFace> faces;
    std::string textureFilePath;

    float minPositionX, maxPositionX;
    float minPositionY, maxPositionY;
    float minPositionZ, maxPositionZ;
};