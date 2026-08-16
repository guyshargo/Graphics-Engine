#include "PathResolver.h"
#include <SDL.h>
#include <filesystem>
#include <algorithm>

namespace PathResolver {

    std::string GetExecutableDir() {
        char* basePath = SDL_GetBasePath();
        if (basePath) {
            std::string path(basePath);
            SDL_free(basePath); // Free the memory allocated by SDL
            
            // Standardize slashes for cross-platform safety
            std::replace(path.begin(), path.end(), '\\', '/');
            return path;
        }
        
        // Fallback if SDL fails to determine the base path
        std::string fallback = std::filesystem::current_path().string() + "/";
        std::replace(fallback.begin(), fallback.end(), '\\', '/');
        return fallback;
    }

    std::string ResolvePath(const std::string& relativePath) {
        return GetExecutableDir() + relativePath;
    }
}