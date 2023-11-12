#pragma once

#include "assets/models/VertexFormat.h"

#include "core/containers/Array.h"
#include "core/containers/IndexSpan.h"
#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"

namespace Assets {

struct MeshPart {
    std::string name;
    Core::IndexSpan indices;
};

struct Mesh {
    VertexFormat vertexFormat;
    Core::Array<MeshPart> meshParts;
    Core::Array<std::byte> vertexData;
    Core::Array<uint32_t> indexData;

    void deduplicateVertices();
    void optimizeVertexOrder();
};
}    // namespace Assets

namespace Core::IO {
template <>
struct Serializer<Assets::MeshPart> {
    static void serialize(OutputStream& stream, const Assets::MeshPart& value) {
        stream.write(value.name);
        stream.write(value.indices);
    }
};

template <>
struct Deserializer<Assets::MeshPart> {
    static Assets::MeshPart deserialize(InputStream& stream) {
        auto name    = stream.read<std::string>();
        auto indices = stream.read<IndexSpan>();
        return Assets::MeshPart{.name = std::move(name), .indices = indices};
    }
};

template <>
struct Serializer<Assets::Mesh> {
    static void serialize(OutputStream& stream, const Assets::Mesh& value) {
        stream.write(value.vertexFormat);
        stream.write(value.meshParts);
        stream.write(value.vertexData);
        stream.write(value.indexData);
    }
};

template <>
struct Deserializer<Assets::Mesh> {
    static Assets::Mesh deserialize(InputStream& stream) {
        auto vertexFormat = stream.read<Assets::VertexFormat>();
        auto meshParts    = stream.read<Core::Array<Assets::MeshPart>>();
        auto vertexData   = stream.read<Core::Array<std::byte>>();
        auto indexData    = stream.read<Core::Array<uint32_t>>();

        return Assets::Mesh{.vertexFormat = std::move(vertexFormat),
                            .meshParts    = std::move(meshParts),
                            .vertexData   = std::move(vertexData),
                            .indexData    = std::move(indexData)};
    }
};
}    // namespace Core::IO