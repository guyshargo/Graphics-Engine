#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace PlatformUtils {

    /**
     * @brief Pauses the engine and opens a native operating system window, allowing the user to 
     *        visually browse and select files to load instead of typing folder paths manually.
     * 
     * @param fileExtension The required file extension to filter the dialog (e.g., "obj").
     * @param initialDirectory The default directory path to open the dialog in.
     * @return The absolute path to the selected file, or an empty string if the user canceled.
     */
    std::string openFileChooser(const std::string& fileExtension, const std::string& initialDirectory);

    /**
     * @brief Shortens a full system file path down to just the folder structure inside the engine's 
     *        directory, ensuring saved scenes still load correctly even if the user moves the 
     *        engine to a different hard drive.
     * 
     * @param absolutePath The full system path to the file.
     * @return The computed relative path.
     */
    std::string getRelativePath(const std::string& absolutePath);
}