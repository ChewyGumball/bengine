#pragma once

#include <Renderer/Backends/RendererBackend.h>

#include <memory>

namespace Renderer {

class Renderer {
public:
    Renderer(std::unique_ptr<Backends::RendererBackend> backend) : backend(std::move(backend)) {}

private:
    std::unique_ptr<Backends::RendererBackend> backend;
};

}    // namespace Renderer
