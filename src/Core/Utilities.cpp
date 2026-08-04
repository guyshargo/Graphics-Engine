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

    // Helper for BMP to convert int to 4 bytes
    static void writeIntToStream(std::ofstream& stream, int value) {
        unsigned char bytes[4] = {
            static_cast<unsigned char>(value & 0xFF),
            static_cast<unsigned char>((value >> 8) & 0xFF),
            static_cast<unsigned char>((value >> 16) & 0xFF),
            static_cast<unsigned char>((value >> 24) & 0xFF)
        };
        stream.write(reinterpret_cast<char*>(bytes), 4);
    }

    void saveIntBufferAsBMP(const std::vector<int>& buffer, int width, int height, const std::string& filePath) {
        const int BYTES_PER_PIXEL = 3; // BMP uses 24-bit color
        const int rowPadding = (4 - (width * BYTES_PER_PIXEL) % 4) % 4; // 4-byte boundary padding
        const int imageSize = (width * BYTES_PER_PIXEL + rowPadding) * height; // Total image data size
        const int fileSize = 54 + imageSize; // File size (header + image data)

        std::ofstream fos(filePath, std::ios::binary);
        if (!fos.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return;
        }

        // Write BMP Header (14 bytes)
        fos.write("BM", 2); // Signature
        writeIntToStream(fos, fileSize); // File size
        fos.write("\0\0\0\0", 4); // Reserved
        writeIntToStream(fos, 54); // Data offset

        // Write DIB Header (40 bytes)
        writeIntToStream(fos, 40); // Header size
        writeIntToStream(fos, width); // Image width
        writeIntToStream(fos, height); // Image height
        fos.write("\1\0", 2); // Planes
        fos.write("\x18\0", 2); // Bits per pixel (24)
        fos.write("\0\0\0\0", 4); // Compression
        writeIntToStream(fos, imageSize); // Image size
        fos.write("\0\0\0\0", 4); // X pixels per meter
        fos.write("\0\0\0\0", 4); // Y pixels per meter
        fos.write("\0\0\0\0", 4); // Colors in color table
        fos.write("\0\0\0\0", 4); // Important colors

        // Write Pixel Data
        std::vector<unsigned char> rowData(width * BYTES_PER_PIXEL + rowPadding, 0);
        
        for (int y = height - 1; y >= 0; y--) { // BMP stores pixels bottom-to-top
            int rowIndex = 0;
            for (int x = 0; x < width; x++) {
                int index = y * width + x;
                int argb = buffer[index];
                
                rowData[rowIndex++] = static_cast<unsigned char>(argb & 0xFF);         // Blue
                rowData[rowIndex++] = static_cast<unsigned char>((argb >> 8) & 0xFF);  // Green
                rowData[rowIndex++] = static_cast<unsigned char>((argb >> 16) & 0xFF); // Red
            }
            // Padding is already 0 initialized in the vector, just write the row
            fos.write(reinterpret_cast<const char*>(rowData.data()), rowData.size());
        }

        std::cout << "Image successfully written to " << filePath << std::endl;
    }

    void saveIntBufferAsCSV(const std::vector<int>& buffer, int width, int height, const std::string& filePath) {
        std::FILE* file = std::fopen(filePath.c_str(), "w");
        if (!file) {
            std::cerr << "Failed to open CSV file for writing: " << filePath << std::endl;
            return;
        }

        char textBuffer[32];

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index = y * width + x;
                int argb = buffer[index];

                // Extract RGB components
                int blue = argb & 0xFF;
                int green = (argb >> 8) & 0xFF;
                int red = (argb >> 16) & 0xFF;

                // Write R-G-B using C-style formatting
                std::snprintf(textBuffer, sizeof(textBuffer), "%03d-%03d-%03d", red, green, blue);
                std::fputs(textBuffer, file);

                // Add comma if not the last column
                if (x < width - 1) {
                    std::fputc(',', file);
                }
            }
            // New line after each row
            std::fputc('\n', file);
        }

        std::fclose(file);
        std::cout << "CSV file successfully written to " << filePath << std::endl;
    }
}