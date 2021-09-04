#pragma once

#include <Assets/Buffers/BufferLayout.h>
#include <Assets/Models/VertexFormat.h>

#include <Core/Containers/Array.h>
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
    uint32_t startLocation;
    uint32_t locationCount;
    VertexInputRateType rate;
    Property property;
    VertexUsageName usage;
};

struct ShaderSource {
    Core::Array<std::byte> spirv;
    std::string entryPoint;
};

struct Shader {
    Core::HashMap<PipelineStageType, ShaderSource> stageSources;
    Core::HashMap<std::string, ShaderUniform> uniforms;
    Core::HashMap<std::string, VertexInput> vertexInputs;
    VertexFormat instanceFormat;
};
}    // namespace Assets

namespace Core::IO {

template <>
struct Serializer<Assets::ShaderUniform> {
    static void serialize(OutputStream& stream, const Assets::ShaderUniform& uniform) {
        stream.write(uniform.bindingIndex);
        stream.write(uniform.stage);
        stream.write(uniform.description);
    }
};

template <>
struct Deserializer<Assets::ShaderUniform> {
    static Assets::ShaderUniform deserialize(InputStream& stream) {
        auto binding     = stream.read<uint32_t>();
        auto stage       = stream.read<Assets::PipelineStageType>();
        auto description = stream.read<Assets::ShaderUniformDescription>();

        return Assets::ShaderUniform{
              .bindingIndex = binding,
              .stage        = stage,
              .description  = std::move(description),
        };
    }
};

template <>
struct Serializer<Assets::ShaderSource> {
    static void serialize(OutputStream& stream, const Assets::ShaderSource& source) {
        stream.write(source.spirv);
        stream.write(source.entryPoint);
    }
};

template <>
struct Deserializer<Assets::ShaderSource> {
    static Assets::ShaderSource deserialize(InputStream& stream) {
        auto spirv      = stream.read<Core::Array<std::byte>>();
        auto entryPoint = stream.read<std::string>();

        return Assets::ShaderSource{
              .spirv      = std::move(spirv),
              .entryPoint = std::move(entryPoint),
        };
    }
};

template <>
struct Serializer<Assets::Shader> {
    static void serialize(OutputStream& stream, const Assets::Shader& shader) {
        stream.write(shader.stageSources);
        stream.write(shader.uniforms);
        stream.write(shader.vertexInputs);
        stream.write(shader.instanceFormat);
    }
};

template <>
struct Deserializer<Assets::Shader> {
    static Assets::Shader deserialize(InputStream& stream) {
        auto sources  = stream.read<Core::HashMap<Assets::PipelineStageType, Assets::ShaderSource>>();
        auto uniforms = stream.read<Core::HashMap<std::string, Assets::ShaderUniform>>();
        auto inputs   = stream.read<Core::HashMap<std::string, Assets::VertexInput>>();
        auto format   = stream.read<Assets::VertexFormat>();

        return Assets::Shader{
              .stageSources   = std::move(sources),
              .uniforms       = std::move(uniforms),
              .vertexInputs   = std::move(inputs),
              .instanceFormat = std::move(format),
        };
    }
};
}    // namespace Core::IO
