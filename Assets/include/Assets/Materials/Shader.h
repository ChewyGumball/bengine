#pragma once

#include <Assets/BufferLayout.h>

#include <Core/Containers/HashMap.h>

#include <filesystem>
#include <variant>

namespace Assets {

using PipelineStageType = uint8_t;

namespace PipelineStage {
constexpr PipelineStageType VERTEX      = 0;
constexpr PipelineStageType TESSELATION = 1;
constexpr PipelineStageType GEOMETRY    = 2;
constexpr PipelineStageType FRAGMENT    = 3;

constexpr std::string_view AsString(const PipelineStageType& type) {
    switch(type) {
        case VERTEX: return "vertex";
        case TESSELATION: return "tesselation";
        case GEOMETRY: return "geometry";
        case FRAGMENT: return "fragment";
        default: return "unknown pipeline state";
    }
}
}    // namespace PipelineStage

using BufferDescription = BufferLayout<std::string>;

struct SamplerDescription {
    // sampler type
};

using ShaderUniformDescription = std::variant<BufferDescription, SamplerDescription>;

struct ShaderUniform {
    uint32_t bindingIndex;
    PipelineStageType stage;
    ShaderUniformDescription description;
};

struct VertexInput {
    uint32_t bindingIndex;
    uint32_t location;
    Property property;
};

struct Shader {
    Core::HashMap<PipelineStageType, std::filesystem::path> stageSources;
    Core::HashMap<std::string, ShaderUniform> uniforms;
    Core::HashMap<std::string, VertexInput> vertexInputs;
};
}    // namespace Assets