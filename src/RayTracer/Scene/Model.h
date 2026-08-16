#pragma once
#include <string>
#include <vector>
#include "ModelLight.h"
#include "ModelMaterial.h"
#include "ModelSphere.h"
#include "./Textures/SphereTexture.h"

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

    Model();

    // Constructor
    explicit Model(const std::string& modelFilename);
};