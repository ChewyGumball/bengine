#pragma once

#include "core/geometry/Transform.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Core::Geometry {
struct Plane {
    glm::vec3 normal;
    float distanceToOrigin;

    Plane() = default;
    Plane(glm::vec3 planeNormal, glm::vec3 planePoint);
    explicit Plane(glm::vec4 coefficients);
    Plane(glm::vec3 firstPoint, glm::vec3 secondPoint, glm::vec3 thirdPoint);
    Plane(const Plane& plane, const Transform& transform);
};
}    // namespace Core::Geometry