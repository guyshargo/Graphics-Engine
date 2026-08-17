#include "ParameterSerializer.h"
#include "json.hpp"
#include "PathResolver.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool ParameterSerializer::Load(SavedParams& params, const std::string& filename) {
    std::string fullPath = PathResolver::ResolvePath(filename);
    std::ifstream file(fullPath);
    
    if (!file.is_open()) {
        std::cerr << "Parameter file missing or invalid. Creating default parameter file at: " << fullPath << "\n";
        // If the file is missing, immediately generate a fresh one filled with the hardcoded DefaultParams
        Save(params, filename);
        return false;
    }

    try {
        json j;
        // Parse the raw text file directly into the JSON object structure
        file >> j;

        // Isolate the specific parameter groups to prevent key collisions between the engines
        json rtJson = j.contains("RayTracer") ? j["RayTracer"] : j;
        json rastJson = j.contains("Rasterizer") ? j["Rasterizer"] : j;
        
        // Extract values using fallback defaults if a specific key is missing from an older save file
        params.setModelFileName(EngineMode::RAY_TRACING, rtJson.value("rtModelFileName", DefaultParams::RT_MODEL_FILE_NAME));
        params.setDepthOfRayTracing(rtJson.value("depthOfRayTracing", DefaultParams::DEPTH_OF_RAY_TRACING));
        params.setAntialiasingSamples(rtJson.value("antialiasingSamples", DefaultParams::ANTIALIASING_SAMPLES));
        params.setSoftShadowSamples(rtJson.value("softShadowSamples", DefaultParams::SOFT_SHADOW_SAMPLES));
        params.setAperatureRadius(rtJson.value("aperatureRadius", DefaultParams::APERATURE_RADIUS));
        params.setFocalDistance(rtJson.value("focalDistance", DefaultParams::FOCAL_DISTANCE));
        params.setDisplayType(EngineMode::RAY_TRACING, rtJson.value("rtDisplayType", static_cast<int>(RayTracingDisplayTypeEnum::EX_8_Transparency)));

        params.setModelFileName(EngineMode::RASTERIZATION, rastJson.value("rastModelFileName", DefaultParams::RAST_MODEL_FILE_NAME));
        params.setProjectionType(static_cast<ProjectionTypeEnum>(rastJson.value("projectionType", static_cast<int>(ProjectionTypeEnum::ORTHOGRAPHIC))));
        params.setDisplayType(EngineMode::RASTERIZATION, rastJson.value("rastDisplayType", static_cast<int>(RasterizationDisplayTypeEnum::FACE_EDGES)));
        params.setDisplayNormals(rastJson.value("displayNormals", false));
        
        return true;

    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << "\n";
        return false;
    }
}

bool ParameterSerializer::Save(const SavedParams& params, const std::string& filename) {
    json j;
    
    // Package Ray Tracer settings under an explicit JSON group
    j["RayTracer"]["rtModelFileName"] = params.getRtModelFileName();
    j["RayTracer"]["depthOfRayTracing"] = params.getDepthOfRayTracing();
    j["RayTracer"]["antialiasingSamples"] = params.getAntialiasingSamples();
    j["RayTracer"]["softShadowSamples"] = params.getSoftShadowSamples();
    j["RayTracer"]["aperatureRadius"] = params.getAperatureRadius();
    j["RayTracer"]["focalDistance"] = params.getFocalDistance();
    j["RayTracer"]["rtDisplayType"] = params.getDisplayType(EngineMode::RAY_TRACING);

    // Package Rasterizer settings under an explicit JSON group
    j["Rasterizer"]["rastModelFileName"] = params.getRastModelFileName();
    j["Rasterizer"]["projectionType"] = static_cast<int>(params.getProjectionType());
    j["Rasterizer"]["rastDisplayType"] = static_cast<int>(params.getDisplayType(EngineMode::RASTERIZATION));
    j["Rasterizer"]["displayNormals"] = params.isDisplayNormals();

    std::string fullPath = PathResolver::ResolvePath(filename);
    std::ofstream file(fullPath);
    if (file.is_open()) {
        // Output the JSON object to the text file with a 4-space indent for readability
        file << j.dump(4); 
        return true;
    }
    
    std::cerr << "Could not save parameters to file: " << fullPath << "\n";
    return false;
}