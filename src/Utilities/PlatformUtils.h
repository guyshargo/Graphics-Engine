#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace PlatformUtils {
    std::string openFileChooser(const std::string& fileExtension, const std::string& initialDirectory);
    std::string getRelativePath(const std::string& absolutePath);
}