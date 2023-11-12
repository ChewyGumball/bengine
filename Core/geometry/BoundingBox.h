#pragma once

#include "Core/Geometry/Transform.h"

#include <glm/vec3.hpp>

namespace Core::Geometry {
struct AxisAlignedBoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

struct OrientedBoundingBox {
    AxisAlignedBoundingBox localBounds;
    Transform transform;
};
}    // namespace Core::Geometry