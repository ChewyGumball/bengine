#pragma once

#include "renderer/backends/RendererBackend.h"

#include "assets/materials/Shader.h"
#include "assets/models/Mesh.h"
#include "core/containers/OpaqueID.h"

#include <memory>

namespace Renderer {

using MeshID    = Core::OpaqueID<struct GPUMesh, uint32_t>;
using TextureID = Core::OpaqueID<struct GPUTexture, uint32_t>;

class Renderer {
public:
    Renderer(std::unique_ptr<Backends::RendererBackend> backend) : backend(std::move(backend)) {}

    void drawMesh(const Assets::Mesh& mesh);

    void present();

    void setDefaultMesh(MeshID meshID);
    void setDefaultTexture(TextureID textureID);

private:
    std::unique_ptr<Backends::RendererBackend> backend;
};

}    // namespace Renderer
