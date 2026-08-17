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

# Fetch magic_enum
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG        v0.9.6 # Using the latest stable release
)
FetchContent_MakeAvailable(magic_enum)

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

# Collect all source files using GLOB
file(GLOB CORE_SOURCES "src/Core/*.cpp")
file(GLOB_RECURSE RAYTRACER_SOURCES "src/RayTracer/*.cpp")
file(GLOB_RECURSE RASTERIZER_SOURCES "src/Rasterizer/*.cpp")
file(GLOB Utilities_SOURCES "src/Utilities/*.cpp")

add_executable(GraphicsEngine
    ${CORE_SOURCES}
    ${RAYTRACER_SOURCES}
    ${RASTERIZER_SOURCES}
    ${Utilities_SOURCES}
)

# Tell the engine where to find the header files
target_include_directories(GraphicsEngine PRIVATE
    src/Core
    src/RayTracer
    src/vendor
    src/Rasterizer
    src/Utilities
)

# Link everything
target_link_libraries(GraphicsEngine PRIVATE 
    glm::glm 
    SDL2::SDL2-static
    imgui
    magic_enum::magic_enum
)

# Copy the Models folder to the output directory where the .exe is built
add_custom_command(TARGET GraphicsEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets/RT_Models"
    "$<TARGET_FILE_DIR:GraphicsEngine>/RT_Models"
    
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets/RAST_Models"
    "$<TARGET_FILE_DIR:GraphicsEngine>/RAST_Models"
)