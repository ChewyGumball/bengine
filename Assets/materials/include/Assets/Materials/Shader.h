#pragma once

#include <Assets/Buffers/BufferLayout.h>
#include <Assets/Models/VertexFormat.h>

#include <Core/Containers/HashMap.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/IO/Serialization/OutputStream.h>

#include <filesystem>
#include <variant>

namespace Assets {

using PipelineStageType = uint8_t;

namespace PipelineStage {
constexpr PipelineStageType VERTEX      = 0;
constexpr PipelineStageType TESSELATION = 1;
constexpr PipelineStageType GEOMETRY    = 2;
constexpr PipelineStageType FRAGMENT    = 3;

constexpr std::string_view AsString(const PipelineStageType type) {
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

using VertexInputRateType = uint8_t;
namespace VertexInputRate {
constexpr VertexInputRateType PER_VERTEX   = 0;
constexpr VertexInputRateType PER_INSTANCE = 1;

constexpr std::string_view AsString(const VertexInputRateType type) {
    switch(type) {
        case PER_VERTEX: return "per vertex";
        case PER_INSTANCE: return "per instance";
        default: return "unknown vertex input rate";
    }
}
}    // namespace VertexInputRate

struct VertexInput {
    uint32_t bindingIndex;
    uint32_t location;
    VertexInputRateType rate;
    Property property;
    VertexUsageName usage;
};

struct ShaderSource {
    std::filesystem::path filePath;
    std::string entryPoint;
};

struct Shader {
    Core::HashMap<PipelineStageType, ShaderSource> stageSources;
    Core::HashMap<std::string, ShaderUniform> uniforms;
    Core::HashMap<std::string, VertexInput> vertexInputs;
};
}    // namespace Assets

namespace Core::IO {
template <>
struct Serializer<Assets::Shader> {
    static void serialize(OutputStream& stream, const Assets::Shader& shader) {
        stream.write(shader.stageSources);
        stream.write(shader.uniforms);
        stream.write(shader.vertexInputs);
    }
};
}    // namespace Core::IO
