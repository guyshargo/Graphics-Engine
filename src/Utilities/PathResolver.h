#pragma once
#include <string>

namespace PathResolver {
    
    /**
     * @brief Locates the exact folder on the hard drive where the engine program is currently 
     *        running from, ensuring the software always knows its own location.
     * 
     * @return A string representing the absolute path, standardized with forward slashes.
     */
    std::string GetExecutableDir();

    /**
     * @brief Combines the engine's base folder location with an asset's file name to generate 
     *        the complete system path needed to successfully load textures and 3D models.
     * 
     * @param relativePath The path relative to the executable (e.g., "assets/texture.bmp").
     * @return The combined absolute path string.
     */
    std::string ResolvePath(const std::string& relativePath);
}