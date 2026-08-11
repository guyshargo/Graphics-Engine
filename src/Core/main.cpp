#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <execution>
#include <windows.h> // For the File Open Dialog
#include <filesystem>
#include <SDL.h>
#include <glm/glm.hpp>
#include <magic_enum.hpp>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "RayTracerWorld.h"
#include "RasterizerWorld.h"
#include "SavedParams.h"
#include "DefaultParams.h"
#include "ExerciseEnum.h"
#include "Utilities.h"
#include "Model.h"

// --------------------------------------------------------
// Event Handlers
// --------------------------------------------------------
void handleMouseWheelScrolling(const SDL_Event& event, RasterizerWorld& rasterizerWorld, bool& updateWindowFlag) {
    rasterizerWorld.zoomCamera(static_cast<float>(event.wheel.y));
    updateWindowFlag = true;
}

void handleMouseDragged(const SDL_Event& event, RasterizerWorld& rasterizerWorld, bool& updateWindowFlag) {
    if (event.motion.state & SDL_BUTTON_LMASK) {
        rasterizerWorld.rotateCamera(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
        updateWindowFlag = true;
    }
    else if (event.motion.state & SDL_BUTTON_RMASK) {
        rasterizerWorld.panCamera(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
        updateWindowFlag = true;
    }
}

void handleInputEvents(SDL_Event& event, bool& isRunning, EngineMode currentMode, RasterizerWorld& rasterizerWorld, bool& updateWindowFlag) {
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }

        if (currentMode == EngineMode::RASTERIZATION && !ImGui::GetIO().WantCaptureMouse) {
            if (event.type == SDL_MOUSEWHEEL) {
                handleMouseWheelScrolling(event, rasterizerWorld, updateWindowFlag);
            }
            else if (event.type == SDL_MOUSEMOTION) {
                handleMouseDragged(event, rasterizerWorld, updateWindowFlag);
            }
        }
    }
}

void handleOpenFile(EngineMode& currentMode, SavedParams& params, RayTracerWorld& rayTracerWorld, RasterizerWorld& rasterizerWorld, 
                                bool& isRastLoaded, bool& isRtLoaded, bool& updateWindowFlag) {

    std::string modelsDir = (currentMode == EngineMode::RASTERIZATION) ? std::filesystem::absolute("../../RAST_Models").string() 
                                                                        : std::filesystem::absolute("../../RT_Models").string();

    std::string newPath = Utilities::openFileChooser("model", modelsDir);

    if (!newPath.empty()) {

        std::string relativePath = Utilities::getRelativePath(newPath);
        params.setModelFileName(currentMode, relativePath);

        if (currentMode == EngineMode::RASTERIZATION) {
            isRastLoaded = rasterizerWorld.load(relativePath);
        } else {
            isRtLoaded = rayTracerWorld.load(relativePath);
        }

        updateWindowFlag = true;
    }
}

// --------------------------------------------------------
// UI Rendering Abstraction
// --------------------------------------------------------
void renderUserInterface(EngineMode& currentMode, SavedParams& params, RayTracerWorld& rayTracerWorld, bool& isRtLoaded, RasterizerWorld& rasterizerWorld, bool& isRastLoaded, int& selectedRtExercise, int& selectedRastExercise, bool& updateWindowFlag) {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Engine Controls");
    
    ImGui::Text("Active Engine:");
    if (ImGui::RadioButton("Ray Tracer", (int*)&currentMode, (int)EngineMode::RAY_TRACING)) { updateWindowFlag = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Rasterizer", (int*)&currentMode, (int)EngineMode::RASTERIZATION)) { updateWindowFlag = true; }
    ImGui::Separator();

    if (currentMode == EngineMode::RAY_TRACING) {
        if (ImGui::Button("Open RT Model")) {
            handleOpenFile(currentMode, params, rayTracerWorld, rasterizerWorld, isRastLoaded, isRtLoaded, updateWindowFlag);
        }
        ImGui::TextWrapped("Model: %s", params.getRtModelFileName().c_str());

        std::string currentExerciseName = std::string(magic_enum::enum_name(static_cast<RayTracingExerciseEnum>(selectedRtExercise)));
        if (ImGui::BeginCombo("RT Exercise", currentExerciseName.c_str())) {
            for (const auto& val : magic_enum::enum_values<RayTracingExerciseEnum>()) {
                bool isSelected = (selectedRtExercise == static_cast<int>(val));
                if (ImGui::Selectable(std::string(magic_enum::enum_name(val)).c_str(), isSelected)) {
                    selectedRtExercise = static_cast<int>(val);
                    params.setExercise(currentMode, selectedRtExercise);
                    rayTracerWorld.setExercise(params.getRtExercise());
                    updateWindowFlag = true;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        ImGui::Text("Depth of Field Controls");

        float currentAperture = params.getAperatureRadius();
        if (ImGui::SliderFloat("Aperture Radius", &currentAperture, 0.0f, 1.0f, "%.3f")) {
            params.setAperatureRadius(currentAperture);
            rayTracerWorld.setAperatureRadius(currentAperture);
            updateWindowFlag = true;
        }

        float currentFocalDist = params.getFocalDistance();
        if (ImGui::SliderFloat("Focal Distance", &currentFocalDist, 0.1f, 20.0f, "%.2f")) {
            params.setFocalDistance(currentFocalDist);
            rayTracerWorld.setFocalDistance(currentFocalDist);
            updateWindowFlag = true;
        }

        ImGui::Separator();
        ImGui::Text("Ray Tracing Parameters");

        if (isRtLoaded && rayTracerWorld.getModel() != nullptr && !rayTracerWorld.getModel()->lights.empty()) {
            const int sharedValues[] = {1, 2, 4, 8, 16};
            const char* sharedLabels[] = {"1", "2", "4", "8", "16"};
            const char* comboTitles[] = {"Antialiasing Samples", "Light Radius", "Shadow Samples"};
            bool hasLight = (isRtLoaded && rayTracerWorld.getModel() != nullptr && !rayTracerWorld.getModel()->lights.empty());

            for (int category = 0; category < 3; category++) {
                if (category == 1 && !hasLight) continue;

                int currentVal;
                if (category == 0) currentVal = params.getAntialiasingSamples();
                else if (category == 1) currentVal = static_cast<int>(rayTracerWorld.getModel()->lights[0].radius);
                else currentVal = params.getSoftShadowSamples();

                int currentIndex = 0;
                for (int i = 0; i < 5; i++) {
                    if (currentVal == sharedValues[i]) {
                        currentIndex = i;
                        break;
                    }
                }

                if (ImGui::BeginCombo(comboTitles[category], sharedLabels[currentIndex])) {
                    for (int i = 0; i < 5; i++) {
                        bool isSelected = (currentIndex == i);
                        if (ImGui::Selectable(sharedLabels[i], isSelected)) {
                            if (category == 0) {
                                params.setAntialiasingSamples(sharedValues[i]);
                                rayTracerWorld.setAntialiasingSamples(params.getAntialiasingSamples());
                            } else if (category == 1) {
                                rayTracerWorld.getModel()->lights[0].radius = static_cast<float>(sharedValues[i]);
                            } else {
                                params.setSoftShadowSamples(sharedValues[i]);
                                rayTracerWorld.setSoftShadowSamples(params.getSoftShadowSamples());
                            }
                            updateWindowFlag = true;
                        }
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
        }
    }
    else {
        if (ImGui::Button("Open Rast Model")) {
            handleOpenFile(currentMode, params, rayTracerWorld, rasterizerWorld, isRastLoaded, isRtLoaded, updateWindowFlag);
        }
        ImGui::TextWrapped("Model: %s", params.getRastModelFileName().c_str());

        std::string currentExerciseName = std::string(magic_enum::enum_name(static_cast<RasterizationExerciseEnum>(selectedRastExercise)));
        if (ImGui::BeginCombo("Rast Exercise", currentExerciseName.c_str())) {
            for (const auto& val : magic_enum::enum_values<RasterizationExerciseEnum>()) {
                bool isSelected = (selectedRastExercise == static_cast<int>(val));
                if (ImGui::Selectable(std::string(magic_enum::enum_name(val)).c_str(), isSelected)) {
                    selectedRastExercise = static_cast<int>(val);
                    params.setExercise(currentMode, selectedRastExercise);
                    rasterizerWorld.exercise = params.getRastExercise();
                    updateWindowFlag = true;
                }
            }
            ImGui::EndCombo();
        }

        std::string currentDisplayTypeName = std::string(magic_enum::enum_name(rasterizerWorld.displayType));
        if (ImGui::BeginCombo("Display Type", currentDisplayTypeName.c_str())) {
            for (const auto& val : magic_enum::enum_values<DisplayTypeEnum>()) {
                bool isSelected = (rasterizerWorld.displayType == val);
                if (ImGui::Selectable(std::string(magic_enum::enum_name(val)).c_str(), isSelected)) {
                    rasterizerWorld.displayType = val;
                    params.setDisplayType(val);
                    updateWindowFlag = true;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        ImGui::Text("Projection:");
        if (ImGui::RadioButton("Orthographic", (int*)&rasterizerWorld.projectionType, (int)ProjectionTypeEnum::ORTHOGRAPHIC)) { 
            params.setProjectionType(ProjectionTypeEnum::ORTHOGRAPHIC);
            updateWindowFlag = true; 
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Perspective", (int*)&rasterizerWorld.projectionType, (int)ProjectionTypeEnum::PERSPECTIVE)) { 
            params.setProjectionType(ProjectionTypeEnum::PERSPECTIVE);
            updateWindowFlag = true; 
        }
    
        ImGui::Separator();
        if (ImGui::Checkbox("Show Normals", &rasterizerWorld.displayNormals)) { 
            params.setDisplayNormals(rasterizerWorld.displayNormals);
            updateWindowFlag = true; 
        }
    }
    ImGui::End();
}

// --------------------------------------------------------
// Render Execution
// --------------------------------------------------------
template <typename ClearFunc, typename SetFunc>
void timerRender(EngineMode currentMode, bool& updateWindowFlag, bool isRtLoaded, bool isRastLoaded, 
                 std::vector<uint32_t>& pixelBuffer, std::vector<int>& pixelIndices, 
                 std::mt19937& pixelOrderGenerator, size_t& currentPixelIndex, size_t PIXELS_PER_FRAME,
                 RayTracerWorld& rayTracerWorld, RasterizerWorld& rasterizerWorld, 
                 SDL_Texture* texture, const ClearFunc& clearImage, const SetFunc& setPixel) {
    
    if (updateWindowFlag) {
        if (currentMode == EngineMode::RAY_TRACING) {
            currentPixelIndex = 0;
            std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0xFF323232);
            std::shuffle(pixelIndices.begin(), pixelIndices.end(), pixelOrderGenerator);
            updateWindowFlag = false; 
        } else {
            clearImage();
            if (isRastLoaded) {
                rasterizerWorld.render(clearImage, setPixel);
                SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), DefaultParams::IMAGE_WIDTH * sizeof(uint32_t));
            }
            updateWindowFlag = false;
        }
    }

    // Ray Tracing Processing (Iterative & Parallelized)
    if (currentMode == EngineMode::RAY_TRACING && isRtLoaded && currentPixelIndex < pixelIndices.size()) {
        size_t endPixelIndex = std::min(currentPixelIndex + PIXELS_PER_FRAME, pixelIndices.size());

        // Inline Lambda for rendering pixels concurrently
        std::for_each(std::execution::par, 
                      pixelIndices.begin() + currentPixelIndex, 
                      pixelIndices.begin() + endPixelIndex, 
                      [&pixelBuffer, &rayTracerWorld](int pixelIndex) {
                          int x = pixelIndex % DefaultParams::IMAGE_WIDTH;
                          int y = pixelIndex / DefaultParams::IMAGE_WIDTH;
                          glm::vec3 color = rayTracerWorld.renderPixel(x, y);
                          Utilities::WriteColorToBuffer(pixelBuffer, x, y, color, DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
                      });
        
        currentPixelIndex = endPixelIndex;
        SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), DefaultParams::IMAGE_WIDTH * sizeof(uint32_t));
    }
}

// --------------------------------------------------------
// Main Application Loop
// --------------------------------------------------------
int main() {
    SavedParams params;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Graphics Engine (C++)",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT, 
                                          SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, 
                                             SDL_TEXTUREACCESS_STREAMING, 
                                             DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    EngineMode currentMode = EngineMode::RAY_TRACING;
    bool updateWindowFlag = true; 

    std::vector<uint32_t> pixelBuffer(DefaultParams::IMAGE_WIDTH * DefaultParams::IMAGE_HEIGHT, 0xFF323232);
    std::vector<int> pixelIndices(DefaultParams::IMAGE_WIDTH * DefaultParams::IMAGE_HEIGHT);
    std::iota(pixelIndices.begin(), pixelIndices.end(), 0);
    std::random_device randomSeed;
    std::mt19937 pixelOrderGenerator(randomSeed());
    std::shuffle(pixelIndices.begin(), pixelIndices.end(), pixelOrderGenerator);

    RayTracerWorld rayTracerWorld(DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT, 90.0f);
    bool isRtLoaded = rayTracerWorld.load(params.getRtModelFileName()); 
    rayTracerWorld.setDepthOfRayTracing(params.getDepthOfRayTracing());
    rayTracerWorld.setAntialiasingSamples(params.getAntialiasingSamples());
    rayTracerWorld.setSoftShadowSamples(params.getSoftShadowSamples());
    rayTracerWorld.setAperatureRadius(params.getAperatureRadius());
    rayTracerWorld.setFocalDistance(params.getFocalDistance());
    rayTracerWorld.setExercise(params.getRtExercise());
    int selectedRtExercise = static_cast<int>(params.getRtExercise());

    RasterizerWorld rasterizerWorld(DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
    bool isRastLoaded = rasterizerWorld.load(params.getRastModelFileName());
    rasterizerWorld.exercise = params.getRastExercise();
    int selectedRastExercise = static_cast<int>(params.getRastExercise());
    
    rasterizerWorld.cameraPos = DefaultParams::cameraPos;
    rasterizerWorld.cameraLookAtCenter = DefaultParams::cameraLookAtCenter;
    rasterizerWorld.cameraUp = DefaultParams::cameraUp;
    rasterizerWorld.horizontalFOV = DefaultParams::horizontalFOV;
    rasterizerWorld.modelScale = DefaultParams::modelScale;

    rasterizerWorld.lighting_Diffuse = DefaultParams::lighting_Diffuse;
    rasterizerWorld.lighting_Specular = DefaultParams::lighting_Specular;
    rasterizerWorld.lighting_Ambient = DefaultParams::lighting_Ambient;
    rasterizerWorld.lighting_sHininess = DefaultParams::lighting_sHininess;
    rasterizerWorld.lightPositionWorldCoordinates = DefaultParams::lightPosition;

    rasterizerWorld.projectionType = params.getProjectionType();
    rasterizerWorld.displayType = params.getDisplayType();
    rasterizerWorld.displayNormals = params.isDisplayNormals();

    // Inline Lambda Callbacks
    auto clearImage = [&pixelBuffer]() {
        std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0xFF323232);
    };

    auto setPixel = [&pixelBuffer](int x, int y, const glm::vec3& color) {
        Utilities::WriteColorToBuffer(pixelBuffer, x, y, color, DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
    };

    bool isRunning = true;
    size_t currentPixelIndex = 0;
    const size_t PIXELS_PER_FRAME = 5000;

    // Main Loop
    while (isRunning) {
        SDL_Event event;
        
        handleInputEvents(event, isRunning, currentMode, rasterizerWorld, updateWindowFlag);
        
        renderUserInterface(currentMode, params, rayTracerWorld, isRtLoaded, rasterizerWorld, isRastLoaded, selectedRtExercise, selectedRastExercise, updateWindowFlag);
        
        timerRender(currentMode, updateWindowFlag, isRtLoaded, isRastLoaded, pixelBuffer, pixelIndices, pixelOrderGenerator, currentPixelIndex, PIXELS_PER_FRAME, rayTracerWorld, rasterizerWorld, texture, clearImage, setPixel);
        
        ImGui::Render();
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr); 
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData()); 
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}