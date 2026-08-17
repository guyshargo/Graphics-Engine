#pragma once
#include <string>
#include "DisplayEnum.h"
#include "DefaultParams.h"

/**
 * @brief A centralized container that holds every active setting currently selected in the UI. 
 *        This single object is passed to the rendering engines and used to save user data.
 */
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
    RayTracingDisplayTypeEnum rtDisplayType = RayTracingDisplayTypeEnum::EX_8_Transparency;

    // Rasterizer
    ProjectionTypeEnum projectionType = ProjectionTypeEnum::ORTHOGRAPHIC;
    RasterizationDisplayTypeEnum rastDisplayType = RasterizationDisplayTypeEnum::FACE_EDGES;
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

    int getDisplayType(EngineMode currentMode) const { 
        if (currentMode == EngineMode::RAY_TRACING) {
            return static_cast<int>(rtDisplayType);
        } else {
            return static_cast<int>(rastDisplayType);
        }
    }

    ProjectionTypeEnum getProjectionType() const { return projectionType; }
    RasterizationDisplayTypeEnum getRastDisplayType() const { return rastDisplayType; }
    bool isDisplayNormals() const { return displayNormals; }

    // --- Setters ---
    void setDisplayType(EngineMode currentMode, int displayTypeValue) {
        if (currentMode == EngineMode::RAY_TRACING) {
            rtDisplayType = static_cast<RayTracingDisplayTypeEnum>(displayTypeValue);
        } else {
            rastDisplayType = static_cast<RasterizationDisplayTypeEnum>(displayTypeValue);
        }
    }

    void setProjectionType(ProjectionTypeEnum type) { projectionType = type; }
    void setDisplayNormals(bool display) { displayNormals = display; }

    void setModelFileName(EngineMode currentMode, const std::string& fileName) {
        if (currentMode == EngineMode::RASTERIZATION) rastModelFileName = fileName;
        else rtModelFileName = fileName;
    }

    void setDepthOfRayTracing(int depth) { depthOfRayTracing = depth; }
    void setAntialiasingSamples(int samples) { antialiasingSamples = samples; }
    void setSoftShadowSamples(int samples) { softShadowSamples = samples; }
    void setAperatureRadius(float radius) { aperatureRadius = radius; }
    void setFocalDistance(float distance) { focalDistance = distance; }
};