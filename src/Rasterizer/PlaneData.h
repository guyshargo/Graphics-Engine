#pragma once
#include <glm/glm.hpp>

struct Plane {
    glm::vec3 pointOnPlane;
    glm::vec3 normal;

    Plane(const glm::vec3& point, const glm::vec3 norm) : pointOnPlane(point), normal(norm) {};
};