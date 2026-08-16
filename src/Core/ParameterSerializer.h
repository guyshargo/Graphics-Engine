#pragma once
#include "SavedParams.h"
#include <string>

class ParameterSerializer {
public:
    static bool Load(SavedParams& params, const std::string& filepath = "parameters.json");
    static bool Save(const SavedParams& params, const std::string& filepath = "parameters.json");
};