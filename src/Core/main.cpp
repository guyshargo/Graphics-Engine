#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
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

enum class EngineMode {
    RAY_TRACING,
    RASTERIZATION
};

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

    // --- Ray Tracer Setup ---
    std::vector<int> pixelIndices(DefaultParams::IMAGE_WIDTH * DefaultParams::IMAGE_HEIGHT);
    std::iota(pixelIndices.begin(), pixelIndices.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pixelIndices.begin(), pixelIndices.end(), g);

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

    // Rasterizer Callbacks
    auto clearImage = [&pixelBuffer]() {
        std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0xFF323232);
    };

    auto setPixel = [&pixelBuffer](int x, int y, const glm::vec3& color) {
        if (x < 0 || x >= DefaultParams::IMAGE_WIDTH || y < 0 || y >= DefaultParams::IMAGE_HEIGHT) return;
        
        uint8_t r = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
        uint8_t g_c = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
        
        int flipped_y = DefaultParams::IMAGE_HEIGHT - 1 - y;
        pixelBuffer[flipped_y * DefaultParams::IMAGE_WIDTH + x] = (0xFF << 24) | (r << 16) | (g_c << 8) | b;
    };

    // Main Loop
    bool isRunning = true;
    size_t currentIndex = 0;
    const size_t PIXELS_PER_FRAME = 5000;

    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) isRunning = false;

            // --- Camera Hook ---
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
                currentIndex = 0;
                std::fill(pixelBuffer.begin(), pixelBuffer.end(), 0xFF323232);
                std::shuffle(pixelIndices.begin(), pixelIndices.end(), g);
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

        // Ray Tracing Processing (Iterative)
        if (currentMode == EngineMode::RAY_TRACING && isRtLoaded && currentIndex < pixelIndices.size()) {
            size_t end = std::min(currentIndex + PIXELS_PER_FRAME, pixelIndices.size());
            for (; currentIndex < end; ++currentIndex) {
                int idx = pixelIndices[currentIndex];
                int x = idx % DefaultParams::IMAGE_WIDTH;
                int y = idx / DefaultParams::IMAGE_WIDTH;

                glm::vec3 color = rayTracerWorld.renderPixel(x, y);
                
                uint8_t r = static_cast<uint8_t>(std::clamp(color.r * 255.0f, 0.0f, 255.0f));
                uint8_t g_c = static_cast<uint8_t>(std::clamp(color.g * 255.0f, 0.0f, 255.0f));
                uint8_t b = static_cast<uint8_t>(std::clamp(color.b * 255.0f, 0.0f, 255.0f));
                
                int flipped_y = DefaultParams::IMAGE_HEIGHT - 1 - y;
                pixelBuffer[flipped_y * DefaultParams::IMAGE_WIDTH + x] = (0xFF << 24) | (r << 16) | (g_c << 8) | b;
            }
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