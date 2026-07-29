cmake_minimum_required(VERSION 3.15)
project(Graphics-Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Tell CMake where to download GLM (OpenGL Mathematics) from
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8 # Using a stable release version
)

# Make GLM available to our project
FetchContent_MakeAvailable(glm)

add_executable(GraphicsEngine
    src/main.cpp
    src/WorldModel.cpp
)

# Link GLM to our executable
target_link_libraries(GraphicsEngine PRIVATE glm::glm)