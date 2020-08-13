#pragma once

#include <Core/Status/StatusOr.h>

class GLFWwindow;

namespace GUI {
class Window {
public:
    ~Window();

    static cruise::StatusOr<Window> Create(const std::string& name, uint32_t width, uint32_t height);

private:
    Window(const std::string& name, GLFWwindow* handle);

    std::string name;
    GLFWwindow* handle;
};
}    // namespace GUI