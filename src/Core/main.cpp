#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <execution>
#include <windows.h> // For the File Open Dialog
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
#include "RenderCallbacks.h"

int main() {
    SavedParams params;

    // --- SDL Initialization ---
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

    // --- ImGui Initialization ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // --- State Variables ---
    EngineMode currentMode = EngineMode::RAY_TRACING;
    bool needsRedraw = true; // Tracks when a refresh is required

    std::vector<uint32_t> pixelBuffer(DefaultParams::IMAGE_WIDTH * DefaultParams::IMAGE_HEIGHT, 0xFF323232);

    // --- Setting boudaries of frame to draw
    std::vector<int> pixelIndices(DefaultParams::IMAGE_WIDTH * DefaultParams::IMAGE_HEIGHT);
    std::iota(pixelIndices.begin(), pixelIndices.end(), 0);

    // --- Randomizing pixels order to color ---
    // Provides a random seed
    std::random_device randomSeed;
    // Uses seed to create a pseudo-random generator
    std::mt19937 pixelOrderGenerator(randomSeed());
    // Uses generator to shuffle pixels order
    std::shuffle(pixelIndices.begin(), pixelIndices.end(), pixelOrderGenerator);

    // --- Ray Tracer Setup ---
    RayTracerWorld rayTracerWorld(DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT, 90.0f);
    bool isRtLoaded = rayTracerWorld.load(params.getRtModelFileName()); 
    rayTracerWorld.setRenderingParams(params.getDepthOfRayTracing());
    rayTracerWorld.setExercise(params.getRtExercise());
    int selectedRtExercise = static_cast<int>(params.getRtExercise());

    // --- Rasterizer Setup ---
    RasterizerWorld rasterizerWorld(DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
    bool isRastLoaded = rasterizerWorld.load(params.getRastModelFileName());
    rasterizerWorld.exercise = params.getRastExercise();
    int selectedRastExercise = static_cast<int>(params.getRastExercise());
    
    // --- Camera & Setup variables ---
    rasterizerWorld.cameraPos = DefaultParams::cameraPos;
    rasterizerWorld.cameraLookAtCenter = DefaultParams::cameraLookAtCenter;
    rasterizerWorld.cameraUp = DefaultParams::cameraUp;
    rasterizerWorld.horizontalFOV = DefaultParams::horizontalFOV;
    rasterizerWorld.modelScale = DefaultParams::modelScale;

    // Setup Lighting Defaults
    rasterizerWorld.lighting_Diffuse = DefaultParams::lighting_Diffuse;
    rasterizerWorld.lighting_Specular = DefaultParams::lighting_Specular;
    rasterizerWorld.lighting_Ambient = DefaultParams::lighting_Ambient;
    rasterizerWorld.lighting_sHininess = DefaultParams::lighting_sHininess;
    rasterizerWorld.lightPositionWorldCoordinates = DefaultParams::lightPosition;

    // Setup Render States from JSON
    rasterizerWorld.projectionType = params.getProjectionType();
    rasterizerWorld.displayType = params.getDisplayType();
    rasterizerWorld.displayNormals = params.isDisplayNormals();

    // Initiate render callbacks
    ImageClearer clearImage(pixelBuffer);
    PixelSetter setPixel(pixelBuffer);
    PixelRenderer rendererTask(pixelBuffer, rayTracerWorld);

    // Main Loop
    bool isRunning = true;
    size_t currentPixelIndex = 0;
    const size_t PIXELS_PER_FRAME = 5000;

    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) isRunning = false;

            // --- Camera-Mouse Moevment Hook ---
            // Only process camera inputs if in Rasterization mode and ImGui isn't using the mouse
            if (currentMode == EngineMode::RASTERIZATION && !ImGui::GetIO().WantCaptureMouse) {
                
                if (event.type == SDL_MOUSEWHEEL) {
                    rasterizerWorld.zoomCamera(static_cast<float>(event.wheel.y));
                    needsRedraw = true; // Tell the engine to update the frame
                }
                else if (event.type == SDL_MOUSEMOTION) {
                    
                    // Left click drag to rotate
                    if (event.motion.state & SDL_BUTTON_LMASK) {
                        rasterizerWorld.rotateCamera(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
                        needsRedraw = true;
                    }
                    // Right click drag to pan
                    else if (event.motion.state & SDL_BUTTON_RMASK) {
                        rasterizerWorld.panCamera(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
                        needsRedraw = true;
                    }
                }
            }
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Engine Controls");
        
        // --- Engine Mode Switch ---
        ImGui::Text("Active Engine:");
        if (ImGui::RadioButton("Ray Tracer", (int*)&currentMode, (int)EngineMode::RAY_TRACING)) { needsRedraw = true; }
        ImGui::SameLine();
        if (ImGui::RadioButton("Rasterizer", (int*)&currentMode, (int)EngineMode::RASTERIZATION)) { needsRedraw = true; }
        ImGui::Separator();

        // --- Ray Tracer UI ---
        if (currentMode == EngineMode::RAY_TRACING) {
            if (ImGui::Button("Open RT Model...")) {
                std::string newPath = Utilities::openFileChooser("model", "");
                if (!newPath.empty()) {
                    params.setRtModelFileName(Utilities::getRelativePath(newPath));
                    isRtLoaded = rayTracerWorld.load(params.getRtModelFileName());
                    needsRedraw = true;
                }
            }
            ImGui::TextWrapped("Model: %s", params.getRtModelFileName().c_str());

            std::string currentExerciseName = std::string(magic_enum::enum_name(static_cast<RayTracingExerciseEnum>(selectedRtExercise)));
            if (ImGui::BeginCombo("RT Exercise", currentExerciseName.c_str())) {
                for (const auto& val : magic_enum::enum_values<RayTracingExerciseEnum>()) {
                    bool isSelected = (selectedRtExercise == static_cast<int>(val));
                    if (ImGui::Selectable(std::string(magic_enum::enum_name(val)).c_str(), isSelected)) {
                        selectedRtExercise = static_cast<int>(val);
                        params.setRtExercise(static_cast<RayTracingExerciseEnum>(selectedRtExercise));
                        rayTracerWorld.setExercise(params.getRtExercise());
                        needsRedraw = true;
                    }
                }
                ImGui::EndCombo();
            }
        }
        // --- Rasterizer UI ---
        else {
            if (ImGui::Button("Open Rast Model...")) {
                std::string newPath = Utilities::openFileChooser("obj", "");
                if (!newPath.empty()) {
                    params.setRastModelFileName(Utilities::getRelativePath(newPath));
                    isRastLoaded = rasterizerWorld.load(params.getRastModelFileName());
                    needsRedraw = true;
                }
            }
            ImGui::TextWrapped("Model: %s", params.getRastModelFileName().c_str());

            std::string currentExerciseName = std::string(magic_enum::enum_name(static_cast<RasterizationExerciseEnum>(selectedRastExercise)));
            if (ImGui::BeginCombo("Rast Exercise", currentExerciseName.c_str())) {
                for (const auto& val : magic_enum::enum_values<RasterizationExerciseEnum>()) {
                    bool isSelected = (selectedRastExercise == static_cast<int>(val));
                    if (ImGui::Selectable(std::string(magic_enum::enum_name(val)).c_str(), isSelected)) {
                        selectedRastExercise = static_cast<int>(val);
                        params.setRastExercise(static_cast<RasterizationExerciseEnum>(selectedRastExercise));
                        rasterizerWorld.exercise = params.getRastExercise();
                        needsRedraw = true;
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
                        params.setDisplayType(val); // Save to parameters.json
                        needsRedraw = true;
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Separator();
            ImGui::Text("Projection:");
            if (ImGui::RadioButton("Orthographic", (int*)&rasterizerWorld.projectionType, (int)ProjectionTypeEnum::ORTHOGRAPHIC)) { needsRedraw = true; }
            ImGui::SameLine();
            if (ImGui::RadioButton("Perspective", (int*)&rasterizerWorld.projectionType, (int)ProjectionTypeEnum::PERSPECTIVE)) { needsRedraw = true; }
        
            ImGui::Separator();
            if (ImGui::Checkbox("Show Normals", &rasterizerWorld.displayNormals)) { needsRedraw = true; }
            
        }
        
        ImGui::End();

        // --- Event Execution & Rendering ---
        if (needsRedraw) {
            if (currentMode == EngineMode::RAY_TRACING) {
                currentPixelIndex = 0;
                // Reset the image before starting a new ray-tracing render
                std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0xFF323232);
                // Randomize which pixels are rendered first
                std::shuffle(pixelIndices.begin(), pixelIndices.end(), pixelOrderGenerator);
                needsRedraw = false; 
            } else {
                clearImage();
                if (isRastLoaded) {
                    rasterizerWorld.render(clearImage, setPixel);
                    SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), DefaultParams::IMAGE_WIDTH * sizeof(uint32_t));
                }
                needsRedraw = false;
            }
        }

        // Ray Tracing Processing (Iterative & Parallelized)
        if (currentMode == EngineMode::RAY_TRACING && isRtLoaded && currentPixelIndex < pixelIndices.size()) {
            size_t endPixelIndex = std::min(currentPixelIndex + PIXELS_PER_FRAME, pixelIndices.size());

            // Instantiate the functor with your local variables
            PixelRenderer rendererTask(pixelBuffer, rayTracerWorld);

            // Pass the functor directly to std::for_each
            std::for_each(std::execution::par, 
                          pixelIndices.begin() + currentPixelIndex, 
                          pixelIndices.begin() + endPixelIndex, 
                          rendererTask);
            
            currentPixelIndex = endPixelIndex;
            SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), DefaultParams::IMAGE_WIDTH * sizeof(uint32_t));
        }

        // --- Drawing ---
        ImGui::Render();
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr); 
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData()); 
        SDL_RenderPresent(renderer);
    }

    // --- Cleanup ---
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}