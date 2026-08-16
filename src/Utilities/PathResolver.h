#pragma once
#include <string>

namespace PathResolver {
    // Retrieves the absolute path to the directory containing the executable
    std::string GetExecutableDir();

    // Prepends the executable directory to a given relative path
    std::string ResolvePath(const std::string& relativePath);
}