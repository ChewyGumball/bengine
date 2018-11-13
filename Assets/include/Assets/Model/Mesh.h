#pragma once

#include <Core/Containers/Array.h>
#include <Core/Containers/ArrayView.h>

#include <Core/IO/InputStream.h>
#include <Core/IO/OutputStream.h>

#include "DllExport.h"

#include "VertexFormat.h"

namespace Assets {

struct MeshPart {
    std::string name;
    Core::IndexArrayView indices;
};

struct ASSETS_API Mesh {
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
        return Assets::MeshPart{stream.read<std::string>(), stream.read<IndexArrayView>()};
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
        return Assets::Mesh{stream.read<Assets::VertexFormat>(),
                            stream.read<Core::Array<Assets::MeshPart>>(),
                            stream.read<Core::Array<std::byte>>(),
                            stream.read<Core::Array<uint32_t>>()};
    }
};
}    // namespace Core::IO