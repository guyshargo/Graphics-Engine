#include "Utilities.h"
#include <fstream>
#include <cstdio>
#include <windows.h>
#include <commdlg.h>
namespace Utilities {

    // ==========================================
    // Ray Tracer: Text Parsing Utilities
    // ==========================================
    void parseTokenWithoutParameter(std::istringstream& scanner, const std::string& parameterName) {
        std::string token;
        if (!(scanner >> token)) {
            std::string errorStr = "At parseTokenWithoutParameter, Parsing " + parameterName + ", token is missing.";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        
        std::string expectedFormat = parameterName + ":";
        if (token != expectedFormat) {
            std::string errorStr = "At parseTokenWithoutParameter, Parsing " + parameterName + ", the token " + expectedFormat + " is missing. Found: " + token;
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
    }

    float parseTokenFloat(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        float value;
        if (!(scanner >> value)) {
            std::string errorStr = "At parseTokenFloat, parsing " + parameterName + ", float is missing after token " + parameterName + ":";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        return value;
    }

    int parseTokenInt(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        int value;
        if (!(scanner >> value)) {
            std::string errorStr = "At parseTokenInt, parsing " + parameterName + ", int is missing after token " + parameterName + ":";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        return value;
    }

    std::string parseTokenString(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        std::string value;
        if (!(scanner >> value)) {
            std::string errorStr = "At parseTokenString, parsing " + parameterName + ", string is missing after token " + parameterName + ":";
            std::cerr << errorStr << std::endl;
            throw std::runtime_error(errorStr);
        }
        return value;
    }

    std::string parseTokenRestOfString(std::istringstream& scanner, const std::string& parameterName) {
        parseTokenWithoutParameter(scanner, parameterName);
        
        std::string restOfLine;
        // Read the rest of the string
        std::getline(scanner, restOfLine);
        
        // Trim leading and trailing whitespaces
        size_t start = restOfLine.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return ""; 
        size_t end = restOfLine.find_last_not_of(" \t\r\n");
        
        return restOfLine.substr(start, end - start + 1);
    }


    // ==========================================
    // Rasterizer: File & Image Utilities
    // ==========================================

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

    std::string saveFileChooser(const std::string& fileExtension, const std::string& initialDirectory) {
        OPENFILENAMEA ofn;
        CHAR szFile[MAX_PATH] = { 0 };

        // Set up the default filename "untitled.ext"
        std::string defaultName = "untitled." + fileExtension;
        strncpy_s(szFile, defaultName.c_str(), sizeof(szFile) - 1);

        std::vector<char> filter = createWin32Filter(fileExtension);

        ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = NULL; 
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter.data();
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrDefExt = fileExtension.c_str(); // Auto-appends extension if user forgets
        
        if (!initialDirectory.empty()) {
            ofn.lpstrInitialDir = initialDirectory.c_str();
        }

        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        if (GetSaveFileNameA(&ofn) == TRUE) {
            return std::string(ofn.lpstrFile);
        }

        return "";
    }

    std::string getRelativePath(const std::string& absolutePath) {
        std::filesystem::path currentDir = std::filesystem::current_path();
        std::filesystem::path file(absolutePath);
        
        // Remove the current directory path to get the relative path
        return std::filesystem::relative(file, currentDir).string();
    }
}