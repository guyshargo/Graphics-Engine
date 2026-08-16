#pragma once
#include <SDL.h>
#include <vector>
#include <random>
#include <cstdint>

#include "RayTracerWorld.h"
#include "RasterizerWorld.h"
#include "SavedParams.h"
#include "ExerciseEnum.h"

class Application {
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Shutdown();

private:
    void ProcessEvents();
    void HandleOpenFile();
    void Update();
    void RenderUI();
    void RenderGraphics();

private:
    // Window and rendering context
    SDL_Window* m_Window = nullptr;
    SDL_Renderer* m_Renderer = nullptr;
    SDL_Texture* m_Texture = nullptr;
    
    // Application State
    bool m_IsRunning = false;
    bool m_UpdateWindowFlag = true;
    EngineMode m_CurrentMode = EngineMode::RAY_TRACING;

    // Data buffers
    std::vector<uint32_t> m_PixelBuffer;
    std::vector<int> m_PixelIndices;
    size_t m_CurrentPixelIndex = 0;
    std::mt19937 m_PixelOrderGenerator;

    // Core Engine Systems
    SavedParams m_Params;
    RayTracerWorld m_RayTracerWorld;
    RasterizerWorld m_RasterizerWorld;
    
    bool m_IsRtLoaded = false;
    bool m_IsRastLoaded = false;
    int m_SelectedRtExercise = 0;
    int m_SelectedRastExercise = 0;
};