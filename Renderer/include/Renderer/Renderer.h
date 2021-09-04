#pragma once

#include <Renderer/Backends/RendererBackend.h>

#include <Assets/Materials/Shader.h>
#include <Assets/Models/Mesh.h>

#include <memory>

namespace Renderer {

class Renderer {
public:
    Renderer(std::unique_ptr<Backends::RendererBackend> backend) : backend(std::move(backend)) {}

    void void drawMesh(const Assets::Mesh& mesh);

    void present();

private:
    std::unique_ptr<Backends::RendererBackend> backend;
};

}    // namespace Renderer
