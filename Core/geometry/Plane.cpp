#include "Core/Geometry/Plane.h"

#include <glm/geometric.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Core::Geometry {

Plane::Plane(glm::vec3 planeNormal, glm::vec3 planePoint)
  : normal(glm::normalize(glm::vec3(planeNormal))), distanceToOrigin(glm::dot(-planePoint, normal)) {}
Plane::Plane(glm::vec4 coefficients) : normal(coefficients), distanceToOrigin(coefficients.w) {}
Plane::Plane(glm::vec3 firstPoint, glm::vec3 secondPoint, glm::vec3 thirdPoint)
  : normal(glm::normalize(glm::cross(firstPoint - secondPoint, thirdPoint - secondPoint))),
    distanceToOrigin(glm::dot(-secondPoint, normal)) {}

Plane::Plane(const Plane& plane, const Transform& transform)
  : Plane(glm::rotate(transform.rotation, plane.normal) / transform.scale,
          glm::rotate(transform.rotation, (plane.normal * -plane.distanceToOrigin) * transform.scale) +
                transform.position) {}

}    // namespace Core::Geometry