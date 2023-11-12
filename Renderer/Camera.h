#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Renderer {
struct Camera {
    glm::mat4 projection;

    Camera(glm::mat4 projection) : project(projection) {}

    static Camera
    Perspective(glm::radians fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance) {
        return Camera(glm::perspective(fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance));
    }
    static Camera
    Othographic(float left, float right, float bottom, float top, float nearPlaneDistance, float farPlaneDistance) {
        return Camera(glm::orthographic(left, right, top, bottom, nearPlaneDistance, farPlaneDistance));
    }
};
}    // namespace Renderer