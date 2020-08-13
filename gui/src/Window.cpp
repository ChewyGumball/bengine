#include <GUI/Window.h>

#include <Core/Containers/HashMap.h>
#include <Core/Logging/Logger.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace GUI {

namespace internal {
Core::LogCategory GLFW("glfw");
void GLFWErrorCallback(int error_code, const char* description) {
    Core::Log::Error(GLFW, description);
}

Core::Status InitializeGLFW() {
    static bool glfwInitialized = false;
    if(glfwInitialized) {
        return Core::Status::Ok();
    }

    glfwSetErrorCallback(GLFWErrorCallback);
    int initResult = glfwInit();
    if(initResult == GLFW_FALSE) {
        return Core::Status::Error("GLFW failed to initialize.");
    }

    glfwInitialized = true;
    return Core::Status::Ok();
}

Core::HashMap<GLFWwindow*, bool> WINDOW_RESIZED;
void WindowResizeHandler(GLFWwindow* window, int width, int height) {
    WINDOW_RESIZED[window] = true;
}

}    // namespace internal


Window::Window(const std::string& name, GLFWwindow* handle) : name(name), handle(handle) {}

static Core::StatusOr<Window> Window::Create(const std::string& name, uint32_t width, uint32_t height) {
    RETURN_IF_ERROR(internal::InitializeGLFW());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* handle = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetFramebufferSizeCallback(handle, internal::WindowResizeHandler);

    return Window(handle);
}
}    // namespace GUI
