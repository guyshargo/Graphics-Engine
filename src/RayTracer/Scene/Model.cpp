#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>

#include "./Scene/Model.h"
#include "../Utilities/ParserUtils.h"

// Default constructor
Model::Model() : fovXdegree(0.0f), skyBoxImageFileName(""), comment("") {}

// File parsing constructor
Model::Model(const std::string& modelFilename) : fovXdegree(0.0f) {
    std::ifstream file(modelFilename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open model file: " + modelFilename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // --- FIX 3: Destroy invisible Windows carriage returns (\r) ---
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string lineType = line.substr(0, colonPos);
        std::istringstream lineScanner(line);

        try {
            if (lineType == "comment") {
                comment = ParserUtils::parseTokenRestOfString(lineScanner, "comment");
            } 
            else if (lineType == "fovXdegree") {
                fovXdegree = ParserUtils::parseTokenFloat(lineScanner, "fovXdegree");
            } 
            else if (lineType == "skyBoxImageFileName") {
                skyBoxImageFileName = ParserUtils::parseTokenRestOfString(lineScanner, "skyBoxImageFileName");
                if (!skyBoxImageFileName.empty() && skyBoxImageFileName.back() == '\r') skyBoxImageFileName.pop_back();
            } 
            else if (lineType == "Light") {
                lights.emplace_back(line);
            } 
            else if (lineType == "sphereTextureFileName") {
                std::string sphereTextureFileName = ParserUtils::parseTokenRestOfString(lineScanner, "sphereTextureFileName");
                if (!sphereTextureFileName.empty() && sphereTextureFileName.back() == '\r') sphereTextureFileName.pop_back();
                sphereTextureFileNames.push_back(sphereTextureFileName);
            } 
            else if (lineType == "Material") {
                materials.emplace_back(line);
            } 
            else if (lineType == "Sphere") {
                spheres.emplace_back(line);
            } 
            else {
                throw std::runtime_error("At Model constructor, line type \"" + lineType + "\" not legal.");
            }
        } catch (const std::exception& e) {
            std::string errorMessage = std::string(e.what()) + 
                                       "\nAt Model constructor from modelFilename.\n Failed to parse line:\n" + line;
            std::cerr << errorMessage << std::endl;
            throw std::runtime_error(errorMessage);
        }
    }
    file.close();

    // Get the exact directory where the .model file lives
    std::filesystem::path modelPath(modelFilename);
    std::filesystem::path modelDirectory = modelPath.parent_path();

    if (modelDirectory.empty()) {
        modelDirectory = "Models";
    }

    // --- FIX 2: Resolve Skybox path relative to the model file ---
    if (!skyBoxImageFileName.empty()) {
        std::filesystem::path skyPath(skyBoxImageFileName);
        skyBoxImageFileName = (modelDirectory / skyPath.filename()).string();
    }

    // --- FIX 1: Only ONE loop to load textures, using the corrected paths ---
    for (const std::string& rawTexturePath : sphereTextureFileNames) {
        try {
            std::filesystem::path texPath(rawTexturePath);
            std::string cleanFilename = texPath.filename().string();
            
            std::string finalTexturePath = (modelDirectory / cleanFilename).string();
            
            skyBoxImageSphereTextures.emplace_back(finalTexturePath); 
            
        } catch (const std::exception& e) {
            std::string errorMessage = "At Model constructor.\n Fail to load texture file: \"" + rawTexturePath + "\". Error: " + e.what();
            std::cerr << errorMessage << std::endl;
            throw std::runtime_error(errorMessage);
        }
    }
}