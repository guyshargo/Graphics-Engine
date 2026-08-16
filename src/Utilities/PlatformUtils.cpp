#include "PlatformUtils.h"
#include <algorithm>

#define NOMINMAX
#include <windows.h>
#include <commdlg.h>

namespace PlatformUtils {

    // Helper to format the double-null terminated string required by Win32 API filters
    static std::vector<char> createWin32Filter(const std::string& fileExtension) {
        std::string filterDesc = "Files (*." + fileExtension + ")";
        std::string filterMask = "*." + fileExtension;
        std::vector<char> filter;
        
        filter.insert(filter.end(), filterDesc.begin(), filterDesc.end());
        filter.push_back('\0');
        filter.insert(filter.end(), filterMask.begin(), filterMask.end());
        filter.push_back('\0');
        filter.push_back('\0'); // Final terminator
        
        return filter;
    }

    std::string openFileChooser(const std::string& fileExtension, const std::string& initialDirectory) {
        OPENFILENAMEA ofn;
        CHAR szFile[MAX_PATH] = { 0 };

        std::vector<char> filter = createWin32Filter(fileExtension);

        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = NULL; // No specific owner window
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter.data();
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        
        if (!initialDirectory.empty()) {
            ofn.lpstrInitialDir = initialDirectory.c_str();
        }

        // Flags: File must exist, Path must exist, restore directory after picker closes
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            return std::string(ofn.lpstrFile);
        }

        return ""; 
    }

    std::string getRelativePath(const std::string& absolutePath) {
        std::filesystem::path currentDir = std::filesystem::current_path();
        std::filesystem::path file(absolutePath);
        
        // Remove the current directory path to get the relative path
        std::string relPath = std::filesystem::relative(file, currentDir).string();
        
        // Standardize slashes for cross-platform and JSON safety
        std::replace(relPath.begin(), relPath.end(), '\\', '/');

        return relPath;
    }
}