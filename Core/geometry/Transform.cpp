#include "core/geometry/Transform.h"

namespace Core::Geometry {

Transform::Transform() : rotation(glm::quat()), position(glm::vec3()), scale(glm::vec3(1.0f)){};

bool operator==(const Transform& a, const Transform& b) {
    return a.rotation == b.rotation && a.position == b.position && a.scale == b.scale;
}
}    // namespace Core::Geometry