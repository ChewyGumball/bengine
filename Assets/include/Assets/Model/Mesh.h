#pragma once

#include <Core/Containers/Array.h>
#include <Core/Containers/ArrayView.h>

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
struct Serializer<Assets::Mesh> {
    static void serialize(OutputStream& stream, const Assets::Mesh& value) {
        stream.write(value.vertexFormat);
        stream.write(value.meshParts);
        stream.write(value.vertexData);
        stream.write(value.indexData);
    }
};
}    // namespace Core::IO