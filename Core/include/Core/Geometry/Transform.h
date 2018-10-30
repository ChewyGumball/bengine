#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace Core::Geometry {
struct Transform {
    glm::quat rotation;
    glm::vec3 position;
    glm::vec3 scale;

    Transform();
};

bool operator==(const Transform& a, const Transform& b);
}    // namespace Core::Geometry