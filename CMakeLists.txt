cmake_minimum_required(VERSION 3.15)
project(Graphics-Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch GLM (OpenGL Mathematics)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Fetch SDL2
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.28.5
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
)
FetchContent_MakeAvailable(imgui)

# Library for ImGui including SDL2 backend
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR} 
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PRIVATE SDL2::SDL2-static)

add_executable(GraphicsEngine
    src/main.cpp
    src/WorldModel.cpp
    src/Model.cpp
    src/ModelLight.cpp
    src/ModelMaterial.cpp
    src/ModelSphere.cpp
    src/SphereTexture.cpp
    src/Utilities.cpp
)

# Link everything
target_link_libraries(GraphicsEngine PRIVATE 
    glm::glm 
    SDL2::SDL2-static
    imgui
)