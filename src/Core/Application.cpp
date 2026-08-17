#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "Application.h"
#include "PlatformUtils.h"
#include "ParameterSerializer.h"
#include "ImageUtils.h"
#include "DefaultParams.h"
#include "./Scene/Model.h"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <execution>
#include <filesystem>
#include <glm/glm.hpp>
#include <magic_enum.hpp>

Application::Application() 
    : m_RayTracerWorld(DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT, 90.0f),
      m_RasterizerWorld(DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT) {
}

Application::~Application() {
    Shutdown();
}

bool Application::Init() {
    // Attempt to connect to the operating system's video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    // Spawn the actual window the user will see
    m_Window = SDL_CreateWindow("Graphics Engine (C++)",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT, 
                                SDL_WINDOW_SHOWN);

    // Create the hardware-accelerated drawing context
    m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Allocate a raw block of GPU memory that we will overwrite pixel-by-pixel
    m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, 
                                  SDL_TEXTUREACCESS_STREAMING, 
                                  DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);

    // Initialize the ImGui interface layer
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(m_Window, m_Renderer);
    ImGui_ImplSDLRenderer2_Init(m_Renderer);

    m_CurrentMode = EngineMode::RAY_TRACING;
    m_UpdateWindowFlag = true; 

    // Prepare the raw pixel buffer with a default dark gray background color
    m_PixelBuffer.resize(DefaultParams::IMAGE_WIDTH * DefaultParams::IMAGE_HEIGHT, 0xFF323232);
    m_PixelIndices.resize(DefaultParams::IMAGE_WIDTH * DefaultParams::IMAGE_HEIGHT);
    
    // Create a shuffled list of pixel coordinates so the ray tracer renders pixels in a randomized, progressive scatter pattern
    std::iota(m_PixelIndices.begin(), m_PixelIndices.end(), 0);
    std::random_device randomSeed;
    m_PixelOrderGenerator = std::mt19937(randomSeed());
    std::shuffle(m_PixelIndices.begin(), m_PixelIndices.end(), m_PixelOrderGenerator);

    // Read the user's previous configuration from the JSON file
    ParameterSerializer::Load(m_Params);

    // Inject loaded parameters directly into the Ray Tracer
    m_IsRtLoaded = m_RayTracerWorld.load(m_Params.getRtModelFileName()); 
    m_RayTracerWorld.setDepthOfRayTracing(m_Params.getDepthOfRayTracing());
    m_RayTracerWorld.setAntialiasingSamples(m_Params.getAntialiasingSamples());
    m_RayTracerWorld.setSoftShadowSamples(m_Params.getSoftShadowSamples());
    m_RayTracerWorld.setAperatureRadius(m_Params.getAperatureRadius());
    m_RayTracerWorld.setFocalDistance(m_Params.getFocalDistance());
    m_RayTracerWorld.setDisplayType(static_cast<RayTracingDisplayTypeEnum>(m_Params.getDisplayType(EngineMode::RAY_TRACING)));
    m_SelectedRtDisplay = static_cast<int>(m_Params.getDisplayType(EngineMode::RAY_TRACING));

    // Inject loaded parameters directly into the Rasterizer
    m_IsRastLoaded = m_RasterizerWorld.load(m_Params.getRastModelFileName());
    m_RasterizerWorld.projectionType = m_Params.getProjectionType();
    m_RasterizerWorld.displayType = (static_cast<RasterizationDisplayTypeEnum>(m_Params.getDisplayType(EngineMode::RASTERIZATION)));
    m_RasterizerWorld.displayNormals = m_Params.isDisplayNormals();
    
    // Apply hardcoded camera defaults for the Rasterizer
    m_RasterizerWorld.cameraPos = DefaultParams::cameraPos;
    m_RasterizerWorld.cameraLookAtCenter = DefaultParams::cameraLookAtCenter;
    m_RasterizerWorld.cameraUp = DefaultParams::cameraUp;
    m_RasterizerWorld.horizontalFOV = DefaultParams::HORIZONTAL_FOV;
    m_RasterizerWorld.objectScale = glm::vec3(DefaultParams::MODEL_SCALE);

    m_RasterizerWorld.lighting_Diffuse = DefaultParams::LIGHTING_DIFFUSE;
    m_RasterizerWorld.lighting_Specular = DefaultParams::LIGHTING_SPECULAR;
    m_RasterizerWorld.lighting_Ambient = DefaultParams::LIGHTING_AMBIENT;
    m_RasterizerWorld.lighting_sHininess = DefaultParams::LIGHTING_SHININESS;
    m_RasterizerWorld.lightPositionWorldCoordinates = DefaultParams::lightPosition;

    m_IsRunning = true;
    m_CurrentPixelIndex = 0;
    
    return true;
}

void Application::Run() {
    // The core execution loop of the program
    while (m_IsRunning) {
        ProcessEvents();
        RenderUI();
        RenderGraphics();
        
        ImGui::Render();
        SDL_RenderClear(m_Renderer);

        // Push the fully calculated 2D pixel buffer to the GPU texture
        SDL_RenderCopy(m_Renderer, m_Texture, nullptr, nullptr);

        // Draw the UI panels over the 3D output
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

        // Display the combined frame on the monitor
        SDL_RenderPresent(m_Renderer);
    }
}

void Application::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Forward inputs to ImGui first so it can interact with buttons/sliders
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            m_IsRunning = false;
        }

        // Process direct viewport interaction only if the Rasterizer is active and the mouse is not clicking a UI panel
        if (m_CurrentMode == EngineMode::RASTERIZATION && !ImGui::GetIO().WantCaptureMouse) {
            if (event.type == SDL_MOUSEWHEEL) {
                m_RasterizerWorld.zoomCamera(static_cast<float>(event.wheel.y));
                m_UpdateWindowFlag = true;
            }
            else if (event.type == SDL_MOUSEMOTION) {
                if (event.motion.state & SDL_BUTTON_LMASK) {
                    m_RasterizerWorld.rotateCamera(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
                    m_UpdateWindowFlag = true;
                }
                else if (event.motion.state & SDL_BUTTON_RMASK) {
                    m_RasterizerWorld.panCamera(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
                    m_UpdateWindowFlag = true;
                }
            }
        }
    }
}

void Application::HandleOpenFile() {
    // Determine the starting folder based on which engine is currently active
    std::string modelsDir = (m_CurrentMode == EngineMode::RASTERIZATION) 
        ? std::filesystem::absolute("../../assets/RAST_Models").string() 
        : std::filesystem::absolute("../../assets/RT_Models").string();

    std::string newPath = (m_CurrentMode == EngineMode::RASTERIZATION) 
        ? PlatformUtils::openFileChooser("obj", modelsDir)
        : PlatformUtils::openFileChooser("model", modelsDir);

    if (!newPath.empty()) {
        // Truncate the absolute path to keep configuration files portable across different machines
        std::string relativePath = PlatformUtils::getRelativePath(newPath);
        m_Params.setModelFileName(m_CurrentMode, relativePath);

        if (m_CurrentMode == EngineMode::RASTERIZATION) {
            m_IsRastLoaded = m_RasterizerWorld.load(relativePath);
        } else {
            m_IsRtLoaded = m_RayTracerWorld.load(relativePath);
        }

        // Flag the screen for a complete redraw with the new model
        m_UpdateWindowFlag = true;
    }
}

void Application::RenderUI() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Engine Controls");
    
    ImGui::Text("Active Engine:");
    if (ImGui::RadioButton("Ray Tracer", (int*)&m_CurrentMode, (int)EngineMode::RAY_TRACING)) { m_UpdateWindowFlag = true; }
    ImGui::SameLine();
    if (ImGui::RadioButton("Rasterizer", (int*)&m_CurrentMode, (int)EngineMode::RASTERIZATION)) { m_UpdateWindowFlag = true; }
    ImGui::Separator();

    if (m_CurrentMode == EngineMode::RAY_TRACING) {
        if (ImGui::Button("Open RT Model")) {
            HandleOpenFile();
        }
        ImGui::TextWrapped("Model: %s", m_Params.getRtModelFileName().c_str());

        std::string currentExerciseName = std::string(magic_enum::enum_name(static_cast<RayTracingDisplayTypeEnum>(m_SelectedRtDisplay)));
        if (ImGui::BeginCombo("Display Type:", currentExerciseName.c_str())) {
            for (const auto& val : magic_enum::enum_values<RayTracingDisplayTypeEnum>()) {
                bool isSelected = (m_SelectedRtDisplay == static_cast<int>(val));
                if (ImGui::Selectable(std::string(magic_enum::enum_name(val)).c_str(), isSelected)) {
                    m_SelectedRtDisplay = static_cast<int>(val);
                    m_Params.setDisplayType(EngineMode::RAY_TRACING, m_SelectedRtDisplay);
                    m_RayTracerWorld.setDisplayType(static_cast<RayTracingDisplayTypeEnum>(m_SelectedRtDisplay));
                    m_UpdateWindowFlag = true;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        ImGui::Text("Depth of Field Controls");

        float currentAperture = m_Params.getAperatureRadius();
        if (ImGui::SliderFloat("Aperture Radius", &currentAperture, 0.0f, 1.0f, "%.3f")) {
            m_Params.setAperatureRadius(currentAperture);
            m_RayTracerWorld.setAperatureRadius(currentAperture);
            m_UpdateWindowFlag = true;
        }

        float currentFocalDist = m_Params.getFocalDistance();
        if (ImGui::SliderFloat("Focal Distance", &currentFocalDist, 0.1f, 20.0f, "%.2f")) {
            m_Params.setFocalDistance(currentFocalDist);
            m_RayTracerWorld.setFocalDistance(currentFocalDist);
            m_UpdateWindowFlag = true;
        }

        ImGui::Separator();
        ImGui::Text("Ray Tracing Parameters");

        if (m_IsRtLoaded && m_RayTracerWorld.getModel() != nullptr && !m_RayTracerWorld.getModel()->lights.empty()) {
            const int sharedValues[] = {1, 2, 4, 8, 16};
            const char* sharedLabels[] = {"1", "2", "4", "8", "16"};
            const char* comboTitles[] = {"Antialiasing Samples", "Light Radius", "Shadow Samples"};
            bool hasLight = (m_IsRtLoaded && m_RayTracerWorld.getModel() != nullptr && !m_RayTracerWorld.getModel()->lights.empty());

            for (int category = 0; category < 3; category++) {
                if (category == 1 && !hasLight) continue;

                int currentVal;
                if (category == 0) currentVal = m_Params.getAntialiasingSamples();
                else if (category == 1) currentVal = static_cast<int>(m_RayTracerWorld.getModel()->lights[0].radius);
                else currentVal = m_Params.getSoftShadowSamples();

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
                                m_Params.setAntialiasingSamples(sharedValues[i]);
                                m_RayTracerWorld.setAntialiasingSamples(m_Params.getAntialiasingSamples());
                            } else if (category == 1) {
                                m_RayTracerWorld.getModel()->lights[0].radius = static_cast<float>(sharedValues[i]);
                            } else {
                                m_Params.setSoftShadowSamples(sharedValues[i]);
                                m_RayTracerWorld.setSoftShadowSamples(m_Params.getSoftShadowSamples());
                            }
                            m_UpdateWindowFlag = true;
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
            HandleOpenFile();
        }
        ImGui::TextWrapped("Model: %s", m_Params.getRastModelFileName().c_str());

        std::string currentDisplayTypeName = std::string(magic_enum::enum_name(m_RasterizerWorld.displayType));
        if (ImGui::BeginCombo("Display Type", currentDisplayTypeName.c_str())) {
            for (const auto& val : magic_enum::enum_values<RasterizationDisplayTypeEnum>()) {
                bool isSelected = (m_RasterizerWorld.displayType == val);
                if (ImGui::Selectable(std::string(magic_enum::enum_name(val)).c_str(), isSelected)) {
                    m_RasterizerWorld.displayType = val;
                    m_Params.setDisplayType(EngineMode::RASTERIZATION, static_cast<int>(val));
                    m_UpdateWindowFlag = true;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        ImGui::Text("Projection:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Orthographic", (int*)&m_RasterizerWorld.projectionType, (int)ProjectionTypeEnum::ORTHOGRAPHIC)) { 
            m_Params.setProjectionType(ProjectionTypeEnum::ORTHOGRAPHIC);
            m_UpdateWindowFlag = true; 
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Perspective", (int*)&m_RasterizerWorld.projectionType, (int)ProjectionTypeEnum::PERSPECTIVE)) { 
            m_Params.setProjectionType(ProjectionTypeEnum::PERSPECTIVE);
            m_UpdateWindowFlag = true; 
        }
    
        ImGui::Separator();
        if (ImGui::Checkbox("Show Normals", &m_RasterizerWorld.displayNormals)) { 
            m_Params.setDisplayNormals(m_RasterizerWorld.displayNormals);
            m_UpdateWindowFlag = true; 
        }
        
        ImGui::Separator();
        ImGui::Text("Object Transformations:");
        ImGui::SameLine();
        if (ImGui::Button("Reset Defaults")) {
            m_RasterizerWorld.objectPosition = glm::vec3(0.0f);
            m_RasterizerWorld.objectRotation = glm::vec3(0.0f);
            m_RasterizerWorld.objectScale = glm::vec3(DefaultParams::MODEL_SCALE);
            m_UpdateWindowFlag = true;
        }
        if (ImGui::DragFloat3("Position", &m_RasterizerWorld.objectPosition.x, 0.1f)) { m_UpdateWindowFlag = true; }
        if (ImGui::DragFloat3("Rotation", &m_RasterizerWorld.objectRotation.x, 1.0f)) { m_UpdateWindowFlag = true; }
        if (ImGui::DragFloat3("Scale", &m_RasterizerWorld.objectScale.x, 0.05f)) { m_UpdateWindowFlag = true; }
    }
    ImGui::End();
}

void Application::RenderGraphics() {
    // Wipe the screen and reset tracking variables when a parameter changes
    if (m_UpdateWindowFlag) {
        if (m_CurrentMode == EngineMode::RAY_TRACING) {
            m_CurrentPixelIndex = 0;
            std::fill(m_PixelBuffer.begin(), m_PixelBuffer.end(), 0xFF323232);
            std::shuffle(m_PixelIndices.begin(), m_PixelIndices.end(), m_PixelOrderGenerator);
            m_UpdateWindowFlag = false; 
        } else {
            auto clearImage = [this]() {
                std::fill(m_PixelBuffer.begin(), m_PixelBuffer.end(), 0xFF323232);
            };

            auto setPixel = [this](int x, int y, const glm::vec3& color) {
                ImageUtils::WriteColorToBuffer(m_PixelBuffer, x, y, color, DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
            };

            clearImage();
            if (m_IsRastLoaded) {
                m_RasterizerWorld.render(clearImage, setPixel);

                // Immediately update the texture since rasterization completes in one frame
                SDL_UpdateTexture(m_Texture, nullptr, m_PixelBuffer.data(), DefaultParams::IMAGE_WIDTH * sizeof(uint32_t));
            }
            m_UpdateWindowFlag = false;
        }
    }

    // Process Ray Tracing in iterative chunks to keep the window responsive during heavy calculations
    if (m_CurrentMode == EngineMode::RAY_TRACING && m_IsRtLoaded && m_CurrentPixelIndex < m_PixelIndices.size()) {
        const size_t PIXELS_PER_FRAME = 5000;
        size_t endPixelIndex = std::min(m_CurrentPixelIndex + PIXELS_PER_FRAME, m_PixelIndices.size());

        // Multi-thread the rendering calculations across all available CPU cores
        std::for_each(std::execution::par, 
                        m_PixelIndices.begin() + m_CurrentPixelIndex, 
                        m_PixelIndices.begin() + endPixelIndex, 
                        [this](int pixelIndex) {
                            // Extract the 2D coordinate from the flattened 1D array index
                            int x = pixelIndex % DefaultParams::IMAGE_WIDTH;
                            int y = pixelIndex / DefaultParams::IMAGE_WIDTH;
                            glm::vec3 color = m_RayTracerWorld.renderPixel(x, y);
                            ImageUtils::WriteColorToBuffer(m_PixelBuffer, x, y, color, DefaultParams::IMAGE_WIDTH, DefaultParams::IMAGE_HEIGHT);
                        });
        
        m_CurrentPixelIndex = endPixelIndex;

        // Progressively push the newly calculated pixel chunk to the screen
        SDL_UpdateTexture(m_Texture, nullptr, m_PixelBuffer.data(), DefaultParams::IMAGE_WIDTH * sizeof(uint32_t));
    }
}

void Application::Shutdown() {
    if (m_Renderer || m_Window || m_Texture) {

        // Save current UI state to the JSON file before closing
        ParameterSerializer::Save(m_Params);

        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        // Safely destroy the SDL hardware context to prevent driver memory leaks
        if (m_Texture) SDL_DestroyTexture(m_Texture);
        if (m_Renderer) SDL_DestroyRenderer(m_Renderer);
        if (m_Window) SDL_DestroyWindow(m_Window);
        
        m_Texture = nullptr;
        m_Renderer = nullptr;
        m_Window = nullptr;
        
        SDL_Quit();
    }
}