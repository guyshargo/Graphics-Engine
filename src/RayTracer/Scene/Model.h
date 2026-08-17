#pragma once
#include <string>
#include <vector>
#include "ModelLight.h"
#include "ModelMaterial.h"
#include "ModelSphere.h"
#include "./Textures/SphereTexture.h"

/**
 * @brief Acts as the main container for the entire 3D scene, loading and storing all the 
 *        objects, lights, surface materials, and background images needed to render the final picture.
 */
class Model {
public:
    std::string comment;
    float fovXdegree;
    std::string skyBoxImageFileName;

    std::vector<ModelLight> lights;
    std::vector<std::string> sphereTextureFileNames;
    std::vector<ModelMaterial> materials;
    std::vector<ModelSphere> spheres;
    std::vector<SphereTexture> skyBoxImageSphereTextures;

    // Constructors
    Model();

    /**
     * @brief Parses a system file to populate the scene with data, reading every line to 
     *        instantiate the correct objects and load the associated image files from the hard drive.
     * 
     * @param modelFilename The system path to the scene configuration file.
     */
    explicit Model(const std::string& modelFilename);
};