#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace YourUtilities {

    class YoursUtilities {
    public:
        static glm::mat4 createViewportMatrix(float x, float y, float width, float height);
        static void saveFloatArrayAsBMP(const std::vector<std::vector<float>>& array, const std::string& filename);
        static void saveFloatArrayAsCSV(const std::vector<std::vector<float>>& array, const std::string& filename);
        
    private:
        static void saveFloatArrayAsBMPorCSV(const std::vector<std::vector<float>>& array, const std::string& filename, bool saveAsBmp);
    };

}