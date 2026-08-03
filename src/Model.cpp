#include "Model.h"
#include "Utilities.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

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
        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue; // Or throw an error, depending on how strict you want to be
        }

        std::string lineType = line.substr(0, colonPos);
        std::istringstream lineScanner(line);

        try {
            if (lineType == "comment") {
                comment = Utilities::parseTokenRestOfString(lineScanner, "comment");
            } 
            else if (lineType == "fovXdegree") {
                fovXdegree = Utilities::parseTokenFloat(lineScanner, "fovXdegree");
            } 
            else if (lineType == "skyBoxImageFileName") {
                skyBoxImageFileName = Utilities::parseTokenRestOfString(lineScanner, "skyBoxImageFileName");
            } 
            else if (lineType == "Light") {
                lights.emplace_back(line);
            } 
            else if (lineType == "sphereTextureFileName") {
                std::string sphereTextureFileName = Utilities::parseTokenRestOfString(lineScanner, "sphereTextureFileName");
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

    // Load textures after parsing the file
    for (const std::string& sphereTextureFileName : sphereTextureFileNames) {
        try {
            skyBoxImageSphereTextures.emplace_back(sphereTextureFileName);
        } catch (const std::exception& e) {
            std::string errorMessage = "At Model constructor.\n Fail to load texture file: \"" + sphereTextureFileName + "\". Error: " + e.what();
            std::cerr << errorMessage << std::endl;
            throw std::runtime_error(errorMessage);
        }
    }
}