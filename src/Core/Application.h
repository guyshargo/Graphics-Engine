#pragma once
#include <SDL.h>
#include <vector>
#include <random>
#include <cstdint>

#include "RayTracerWorld.h"
#include "RasterizerWorld.h"
#include "SavedParams.h"
#include "ExerciseEnum.h"

/**
 * @brief Manages the entire application lifecycle, including window creation, user interface, 
 *        mouse/keyboard input, and seamlessly switching between the ray tracing and rasterization engines.
 */
class Application {
public:
    Application();
    ~Application();

    /**
     * @brief Prepares the operating system window, allocates the raw pixel buffer, 
     *        and loads the user's previous UI preferences from the hard drive.
     * 
     * @return True if the graphics context and window successfully initialized, false if a critical failure occurred.
     */
    bool Init();

    /**
     * @brief The continuous loop that keeps the program open, processes user interactions, 
     *        and pushes the final calculated pixel colors to the monitor every frame.
     */
    void Run();

    /**
     * @brief Saves the current UI preferences to a configuration file on the hard drive 
     *        and safely releases all memory and window resources back to the operating system.
     */
    void Shutdown();

private:
    /**
     * @brief Listens for operating system events, specifically capturing mouse clicks, 
     *        scrolls, and window close requests to update the camera or exit the program.
     */
    void ProcessEvents();

    /**
     * @brief Pauses the engine to open a native file browser, allowing the user to select 
     *        a new 3D model file, which is then loaded into the currently active rendering engine.
     */
    void HandleOpenFile();

    /**
     * @brief Draws the graphical menus and buttons on top of the 3D scene, linking the 
     *        UI inputs directly to the engine's settings variables.
     */
    void RenderUI();

    /**
     * @brief Triggers the active graphics engine to calculate the colors for the pixel buffer. 
     *        It manages parallel processing for the ray tracer and standard drawing for the rasterizer.
     */
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
};