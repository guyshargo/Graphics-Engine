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
    std::string modelFileName;
    std::string saveImagePath;
    int depthOfRayTracing;
    ExerciseEnum exercise;

    void setDefaultParams() {
        modelFileName = DefaultParams::MODEL_FILE_NAME;
        saveImagePath = DefaultParams::SAVE_IMAGE_PATH;
        depthOfRayTracing = DefaultParams::DEPTH_OF_RAY_TRACING;
        exercise = static_cast<ExerciseEnum>(0); // Default fallback
    }

    bool loadFromFile() {
        std::ifstream file("parameters.json");
        if (!file.is_open()) return false;

        try {
            json j;
            file >> j; // Magically parses the entire file into the JSON object
            
            // The .value() function tries to read the key, but falls back to the default if missing
            modelFileName = j.value("modelFileName", DefaultParams::MODEL_FILE_NAME);
            saveImagePath = j.value("saveImagePath", DefaultParams::SAVE_IMAGE_PATH);
            depthOfRayTracing = j.value("depthOfRayTracing", DefaultParams::DEPTH_OF_RAY_TRACING);
            
            // Enums are easily stored and read as integers
            exercise = static_cast<ExerciseEnum>(j.value("exercise", 0)); // Default to first exercise if missing
            
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
        j["modelFileName"] = modelFileName;
        j["saveImagePath"] = saveImagePath;
        j["depthOfRayTracing"] = depthOfRayTracing;
        j["exercise"] = static_cast<int>(exercise); 

        std::ofstream file("parameters.json");
        if (file.is_open()) {
            // dump(4) automatically formats the JSON with a beautiful 4-space indent
            file << j.dump(4); 
            return true;
        }
        
        std::cerr << "Could not save parameters to file.\n";
        return false;
    }

    // --- Getters ---
    // Returning strings by const reference prevents expensive string copies
    const std::string& getModelFileName() const { return modelFileName; }
    const std::string& getSaveImagePath() const { return saveImagePath; }
    int getDepthOfRayTracing() const { return depthOfRayTracing; }
    ExerciseEnum getExercise() const { return exercise; }

    // --- Setters ---
    void setModelFileName(const std::string& name) {
        modelFileName = name;
        saveToFile();
    }

    void setSaveImagePath(const std::string& path) {
        saveImagePath = path;
        saveToFile();
    }

    void setDepthOfRayTracing(int depth) {
        depthOfRayTracing = depth;
        saveToFile();
    }

    void setExercise(ExerciseEnum ex) {
        exercise = ex;
        saveToFile();
    }
};