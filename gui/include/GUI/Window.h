#pragma once

#include <Core/Containers/HashSet.h>
#include <Core/Status/StatusOr.h>

#include <Renderer/Backends/Vulkan/VulkanInstance.h>

class GLFWwindow;

namespace GUI {
class Window {
public:
    ~Window();

    static Core::StatusOr<std::unique_ptr<Window>> Create(const std::string& name, uint32_t width, uint32_t height);

    GLFWwindow* getHandle() {
        return handle;
    }

    VkSurfaceKHR createSurface(Renderer::Backends::Vulkan::VulkanInstance& instance);
    Core::HashSet<std::string> getRequiredVulkanExtensions();

    bool hasResized() const;
    VkExtent2D getSize() const;

    bool shouldClose() const;
    void pumpEvents();

private:
    Window(const std::string& name, GLFWwindow* handle);

    std::string name;
    GLFWwindow* handle;
};
}    // namespace GUI