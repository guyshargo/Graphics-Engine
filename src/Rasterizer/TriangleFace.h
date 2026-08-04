#pragma once
#include <glm/glm.hpp>
#include <array>
#include <string>
#include <sstream>

struct TriangleFace {
    std::array<int, 3> indices;
    glm::vec3 color;
    glm::vec3 normal;

    TriangleFace(const std::array<int, 3>& indicesList, const glm::vec3& col, const glm::vec3& norm = glm::vec3(0.0f)) 
                : indices(indicesList), color(col), normal(norm) {}

    std::string toString() const {
        std::ostringstream oss;
        oss << "TriangleFace [indices=" << indices[0] << ", " << indices[1] << ", " << indices[2] 
            << ", color=" << color.x << "," << color.y << "," << color.z << "]";
        return oss.str();
    }
};