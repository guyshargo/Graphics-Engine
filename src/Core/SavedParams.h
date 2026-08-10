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

    // RayTracer - Direct Initialization
    std::string rtModelFileName = DefaultParams::RT_MODEL_FILE_NAME;
    std::string rastModelFileName = DefaultParams::RAST_MODEL_FILE_NAME;
    int depthOfRayTracing = DefaultParams::DEPTH_OF_RAY_TRACING;
    int antialiasingSamples = DefaultParams::ANTIALIASING_SAMPLES;
    int softShadowSamples = DefaultParams::SOFT_SHADOW_SAMPLES;
    float aperatureRadius = DefaultParams::APERATURE_RADIUS;
    float focalDistance = DefaultParams::FOCAL_DISTANCE;
    RayTracingExerciseEnum rtExercise = static_cast<RayTracingExerciseEnum>(0);

    // Rasterizer - Direct Initialization
    RasterizationExerciseEnum rastExercise = static_cast<RasterizationExerciseEnum>(0);
    ProjectionTypeEnum projectionType = ProjectionTypeEnum::ORTHOGRAPHIC;
    DisplayTypeEnum displayType = DisplayTypeEnum::FACE_EDGES;
    bool displayNormals = false;


    bool loadFromFile() {
        std::ifstream file("parameters.json");
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j; // parses the entire file into the JSON object

            json rtJson = j.contains("RayTracer") ? j["RayTracer"] : j;
            json rastJson = j.contains("Rasterizer") ? j["Rasterizer"] : j;
            
            rtModelFileName = rtJson.value("rtModelFileName", DefaultParams::RT_MODEL_FILE_NAME);
            depthOfRayTracing = rtJson.value("depthOfRayTracing", DefaultParams::DEPTH_OF_RAY_TRACING);
            antialiasingSamples = rtJson.value("antialiasingSamples", DefaultParams::ANTIALIASING_SAMPLES);
            softShadowSamples = rtJson.value("softShadowSamples", DefaultParams::SOFT_SHADOW_SAMPLES);
            aperatureRadius = rtJson.value("aperatureRadius", DefaultParams::APERATURE_RADIUS);
            focalDistance = rtJson.value("focalDistance", DefaultParams::FOCAL_DISTANCE);
            rtExercise = static_cast<RayTracingExerciseEnum>(rtJson.value("rtExercise", 0));

            rastModelFileName = rastJson.value("rastModelFileName", DefaultParams::RAST_MODEL_FILE_NAME);
            rastExercise = static_cast<RasterizationExerciseEnum>(rastJson.value("rastExercise", 0));
            projectionType = static_cast<ProjectionTypeEnum>(rastJson.value("projectionType", static_cast<int>(ProjectionTypeEnum::ORTHOGRAPHIC)));
            displayType = static_cast<DisplayTypeEnum>(rastJson.value("displayType", static_cast<int>(DisplayTypeEnum::FACE_EDGES)));
            displayNormals = rastJson.value("displayNormals", false);
            
            return true;

        } catch (const json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << "\n";
            return false; // Fall back to the direct initializations defined above
        }
    }

public:
    SavedParams() {
        if (!loadFromFile()) {
            std::cerr << "Parameter file missing or invalid. Creating default parameter file.\n";
            saveToFile();
        }
    }

    bool saveToFile() const {
        json j;
        
        // --- Ray Tracer Parameters Block ---
        j["RayTracer"]["rtModelFileName"] = rtModelFileName;
        j["RayTracer"]["depthOfRayTracing"] = depthOfRayTracing;
        j["RayTracer"]["antialiasingSamples"] = antialiasingSamples;
        j["RayTracer"]["softShadowSamples"] = softShadowSamples;
        j["RayTracer"]["aperatureRadius"] = aperatureRadius;
        j["RayTracer"]["focalDistance"] = focalDistance;
        j["RayTracer"]["rtExercise"] = static_cast<int>(rtExercise);

        // --- Rasterizer Parameters Block ---
        j["Rasterizer"]["rastModelFileName"] = rastModelFileName;
        j["Rasterizer"]["projectionType"] = static_cast<int>(projectionType);
        j["Rasterizer"]["displayType"] = static_cast<int>(displayType);
        j["Rasterizer"]["displayNormals"] = displayNormals;
        j["Rasterizer"]["rastExercise"] = static_cast<int>(rastExercise);

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
    int getAntialiasingSamples() const { return antialiasingSamples; }
    int getSoftShadowSamples() const { return softShadowSamples; }
    float getAperatureRadius() const { return aperatureRadius; }
    float getFocalDistance() const { return focalDistance; }
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

    void setAntialiasingSamples(int samples) {
        antialiasingSamples = samples;
        saveToFile();
    }

    void setSoftShadowSamples(int samples) {
        softShadowSamples = samples;
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

    void setAperatureRadius(float radius) {
        aperatureRadius = radius;
        saveToFile();
    }

    void setFocalDistance(float distance) {
        focalDistance = distance;
        saveToFile();
    }
};