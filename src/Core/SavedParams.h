#pragma once
#include <string>
#include "ExerciseEnum.h"
#include "DefaultParams.h"

class SavedParams {
private:
    // RayTracer
    std::string rtModelFileName = DefaultParams::RT_MODEL_FILE_NAME;
    std::string rastModelFileName = DefaultParams::RAST_MODEL_FILE_NAME;
    int depthOfRayTracing = DefaultParams::DEPTH_OF_RAY_TRACING;
    int antialiasingSamples = DefaultParams::ANTIALIASING_SAMPLES;
    int softShadowSamples = DefaultParams::SOFT_SHADOW_SAMPLES;
    float aperatureRadius = DefaultParams::APERATURE_RADIUS;
    float focalDistance = DefaultParams::FOCAL_DISTANCE;
    RayTracingExerciseEnum rtExercise = static_cast<RayTracingExerciseEnum>(0);

    // Rasterizer
    RasterizationExerciseEnum rastExercise = static_cast<RasterizationExerciseEnum>(0);
    ProjectionTypeEnum projectionType = ProjectionTypeEnum::ORTHOGRAPHIC;
    DisplayTypeEnum displayType = DisplayTypeEnum::FACE_EDGES;
    bool displayNormals = false;

public:
    SavedParams() = default;

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
    void setProjectionType(ProjectionTypeEnum type) { projectionType = type; }
    void setDisplayType(DisplayTypeEnum type) { displayType = type; }
    void setDisplayNormals(bool display) { displayNormals = display; }

    void setModelFileName(EngineMode currentMode, const std::string& fileName) {
        if (currentMode == EngineMode::RASTERIZATION) rastModelFileName = fileName;
        else rtModelFileName = fileName;
    }

    void setExercise(EngineMode currentMode, int exerciseValue) {
        if (currentMode == EngineMode::RASTERIZATION)
            rastExercise = static_cast<RasterizationExerciseEnum>(exerciseValue);
        else
            rtExercise = static_cast<RayTracingExerciseEnum>(exerciseValue);
    }

    void setDepthOfRayTracing(int depth) { depthOfRayTracing = depth; }
    void setAntialiasingSamples(int samples) { antialiasingSamples = samples; }
    void setSoftShadowSamples(int samples) { softShadowSamples = samples; }
    void setAperatureRadius(float radius) { aperatureRadius = radius; }
    void setFocalDistance(float distance) { focalDistance = distance; }
};