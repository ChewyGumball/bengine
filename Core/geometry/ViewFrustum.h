#pragma once

#include "core/geometry/Plane.h"
#include "core/geometry/Transform.h"

#include "core/containers/Array.h"

#include <glm/mat4x4.hpp>

namespace Core::Geometry {
struct ViewFrustum {
    static constexpr uint32_t Near   = 0;
    static constexpr uint32_t Far    = 1;
    static constexpr uint32_t Left   = 2;
    static constexpr uint32_t Right  = 3;
    static constexpr uint32_t Bottom = 4;
    static constexpr uint32_t Top    = 5;

    Core::FixedArray<Plane, 6> planes;

    ViewFrustum(const glm::mat4& projection, const Transform& transform = Transform());
    ViewFrustum(const ViewFrustum& frustum, const Transform& transform);
};
}    // namespace Core::Geometry