#pragma once
#include "SavedParams.h"
#include <string>

/**
 * @brief Reads and writes the user's UI settings to a JSON text file, ensuring their 
 *        preferences are remembered the next time the engine opens.
 */
class ParameterSerializer {
public:
    /**
     * @brief Parses a JSON file to inject saved UI settings into the application state.
     * 
     * @param params The settings container to populate with loaded data.
     * @param filepath The system path to the saved configuration file.
     * @return True if the file was successfully read and parsed, false if it failed or fell back to defaults.
     */
    static bool Load(SavedParams& params, const std::string& filepath = "parameters.json");

    /**
     * @brief Converts the current application state into JSON format and writes it to the hard drive.
     * 
     * @param params The current settings container to extract data from.
     * @param filepath The system path where the configuration file should be written.
     * @return True if the file successfully wrote to the disk, false otherwise.
     */
    static bool Save(const SavedParams& params, const std::string& filepath = "parameters.json");
};