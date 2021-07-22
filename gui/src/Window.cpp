#include <GUI/Window.h>

#include <Core/Containers/HashMap.h>
#include <Core/Logging/Logger.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <mutex>
#include <shared_mutex>

namespace GUI {

namespace internal {
Core::LogCategory GLFW("glfw");

uint32_t glfwInitializationCount = 0;
std::mutex glfwInitializationMutex;

void GLFWErrorCallback(int error_code, const char* description) {
    Core::Log::Error(GLFW, description);
}

Core::Status InitializeGLFW() {
    std::scoped_lock lock(glfwInitializationMutex);
    if(glfwInitializationCount > 0) {
        return Core::Status::Ok();
    }

    glfwSetErrorCallback(GLFWErrorCallback);
    int initResult = glfwInit();
    if(initResult == GLFW_FALSE) {
        return Core::Status::Error("GLFW failed to initialize.");
    }

    glfwInitializationCount++;
    return Core::Status::Ok();
}

void TerminateGLFW() {
    std::scoped_lock lock(glfwInitializationMutex);
    glfwInitializationCount--;

    if(glfwInitializationCount == 0) {
        glfwTerminate();
    }
}

std::shared_mutex glfwWindowResizedMutex;
Core::HashMap<GLFWwindow*, bool> WINDOW_RESIZED;
void WindowResizeHandler(GLFWwindow* window, int width, int height) {
    std::unique_lock lock(glfwWindowResizedMutex);
    WINDOW_RESIZED[window] = true;
}

}    // namespace internal


Window::Window(const std::string& name, GLFWwindow* handle) : name(name), handle(handle) {
    clearResized();
}

Window::~Window() {
    Core::Log::Info(internal::GLFW, "Destroying window.");
    glfwDestroyWindow(handle);
    internal::TerminateGLFW();
}

VkSurfaceKHR Window::createSurface(Renderer::Backends::Vulkan::VulkanInstance& instance) {
    VkSurfaceKHR surface;
    VK_CHECK(glfwCreateWindowSurface(instance, handle, nullptr, &surface));
    return surface;
}

bool Window::hasResized() const {
    std::shared_lock lock(internal::glfwWindowResizedMutex);
    return internal::WINDOW_RESIZED[handle];
}

void Window::clearResized() {
    std::unique_lock lock(internal::glfwWindowResizedMutex);
    internal::WINDOW_RESIZED[handle] = false;
}

VkExtent2D Window::getSize() const {
    int32_t width, height;
    glfwGetFramebufferSize(handle, &width, &height);

    return VkExtent2D{.width = static_cast<uint32_t>(width), .height = static_cast<uint32_t>(height)};
}

Core::HashSet<std::string> Window::getRequiredVulkanExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    Core::HashSet<std::string> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.insert(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    extensions.insert(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    return extensions;
}
bool Window::shouldClose() const {
    return glfwWindowShouldClose(handle) == GLFW_TRUE;
}

void Window::pumpEvents() {
    glfwPollEvents();
}

Core::StatusOr<std::unique_ptr<Window>> Window::Create(const std::string& name, uint32_t width, uint32_t height) {
    RETURN_IF_ERROR(internal::InitializeGLFW());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* handle = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetFramebufferSizeCallback(handle, internal::WindowResizeHandler);

    return std::unique_ptr<Window>(new Window(name, handle));
}
}    // namespace GUI
