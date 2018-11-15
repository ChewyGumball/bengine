#pragma once

#include <Core/Containers/HashMap.h>

namespace Assets {
enum PipelineStage { Vertex, Tesselation, Geometry, Fragment };
struct Shader {
    PipelineStage stage;
    Core::HashMap<uint32_t, 
};
}    // namespace Assets