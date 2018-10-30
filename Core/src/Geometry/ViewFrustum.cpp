#include "Core/Geometry/ViewFrustum.h"

#include "Core/Algorithms/Containers.h"

namespace Core::Geometry {

ViewFrustum::ViewFrustum(const glm::mat4& projection, const Transform& transform) {
    Core::FixedArray<Plane, 6> unorientedPlanes;

    unorientedPlanes[Left]   = Plane(projection[3] + projection[0]);
    unorientedPlanes[Right]  = Plane(projection[3] - projection[0]);
    unorientedPlanes[Bottom] = Plane(projection[3] + projection[1]);
    unorientedPlanes[Top]    = Plane(projection[3] - projection[1]);
    unorientedPlanes[Near]   = Plane(projection[3] + projection[2]);
    unorientedPlanes[Far]    = Plane(projection[3] - projection[2]);

    if(transform == Transform()) {
        planes = unorientedPlanes;
    } else {
        Core::Algorithms::Map(
              unorientedPlanes, planes, [&transform](const Plane& before) { return Plane(before, transform); });
    }
}

ViewFrustum::ViewFrustum(const ViewFrustum& frustum, const Transform& transform) {
    Core::Algorithms::Map(
          frustum.planes, planes, [&transform](const Plane& before) { return Plane(before, transform); });
}
}    // namespace Core::Geometry