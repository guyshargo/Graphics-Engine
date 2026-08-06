#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include "ExerciseEnum.h"
#include "json.hpp"
#include "DefaultParams.h"

// For convenience, we alias the namespace
using json = nlohmann::json;

class SavedParams {
private:

    // RayTracer
    std::string rtModelFileName;
    std::string rastModelFileName;
    int depthOfRayTracing;
    RayTracingExerciseEnum rtExercise;

    // Rasterizer
    RasterizationExerciseEnum rastExercise;
    ProjectionTypeEnum projectionType;
    DisplayTypeEnum displayType;
    bool displayNormals;

    void setDefaultParams() {
        rtModelFileName = DefaultParams::RT_MODEL_FILE_NAME;
        rastModelFileName = DefaultParams::RAST_MODEL_FILE_NAME;
        depthOfRayTracing = DefaultParams::DEPTH_OF_RAY_TRACING;
        rtExercise = static_cast<RayTracingExerciseEnum>(0);

        projectionType = ProjectionTypeEnum::ORTHOGRAPHIC; 
        displayType = DisplayTypeEnum::FACE_EDGES; 
        displayNormals = false;
        rastExercise = static_cast<RasterizationExerciseEnum>(0);
    }

    bool loadFromFile() {
        std::ifstream file("parameters.json");
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j; // parses the entire file into the JSON object
            
            rtModelFileName = j.value("rtModelFileName", DefaultParams::RT_MODEL_FILE_NAME);
            rastModelFileName = j.value("rastModelFileName", DefaultParams::RAST_MODEL_FILE_NAME);
            depthOfRayTracing = j.value("depthOfRayTracing", DefaultParams::DEPTH_OF_RAY_TRACING);
            rtExercise = static_cast<RayTracingExerciseEnum>(j.value("rtExercise", 0)); 

            rastExercise = static_cast<RasterizationExerciseEnum>(j.value("rastExercise", 0));
            projectionType = static_cast<ProjectionTypeEnum>(j.value("projectionType", static_cast<int>(ProjectionTypeEnum::ORTHOGRAPHIC)));
            displayType = static_cast<DisplayTypeEnum>(j.value("displayType", static_cast<int>(DisplayTypeEnum::FACE_EDGES)));
            displayNormals = j.value("displayNormals", false);
            
            return true;

        } catch (const json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << "\n";
            return false;
        }
    }

public:
    SavedParams() {
        setDefaultParams();
        if (!loadFromFile()) {
            std::cerr << "Parameter file missing or invalid. Creating default parameter file.\n";
            setDefaultParams();
            saveToFile();
        }
    }

    bool saveToFile() const {
        json j;
        j["rtModelFileName"] = rtModelFileName;
        j["rastModelFileName"] = rastModelFileName;
        j["depthOfRayTracing"] = depthOfRayTracing;
        j["rtExercise"] = static_cast<int>(rtExercise);

        j["projectionType"] = static_cast<int>(projectionType);
        j["displayType"] = static_cast<int>(displayType);
        j["displayNormals"] = displayNormals;
        j["rastExercise"] = static_cast<int>(rastExercise);

        std::ofstream file("parameters.json");
        if (file.is_open()) {
            // automatically formats the JSON with a 4-space indent
            file << j.dump(4); 
            return true;
        }
        
        std::cerr << "Could not save parameters to file.\n";
        return false;
    }

    // --- Getters ---
    const std::string& getRtModelFileName() const { return rtModelFileName; }
    const std::string& getRastModelFileName() const { return rastModelFileName; }
    int getDepthOfRayTracing() const { return depthOfRayTracing; }
    RayTracingExerciseEnum getRtExercise() const { return rtExercise; }

    RasterizationExerciseEnum getRastExercise() const { return rastExercise; }
    ProjectionTypeEnum getProjectionType() const { return projectionType; }
    DisplayTypeEnum getDisplayType() const { return displayType; }
    bool isDisplayNormals() const { return displayNormals; }

    // --- Setters ---
    void setProjectionType(ProjectionTypeEnum type) {
        projectionType = type;
        saveToFile();
    }
    
    void setDisplayType(DisplayTypeEnum type) {
        displayType = type;
        saveToFile();
    }

    void setDisplayNormals(bool display) {
        displayNormals = display;
        saveToFile();
    }

    void setRtModelFileName(const std::string& name) {
        rtModelFileName = name;
        saveToFile();
    }

    void setRastModelFileName(const std::string& name) {
        rastModelFileName = name;
        saveToFile();
    }

    void setDepthOfRayTracing(int depth) {
        depthOfRayTracing = depth;
        saveToFile();
    }

    void setRtExercise(RayTracingExerciseEnum ex) {
        rtExercise = ex;
        saveToFile();
    }

    void setRastExercise(RasterizationExerciseEnum ex) {
        rastExercise = ex;
        saveToFile();
    }
};