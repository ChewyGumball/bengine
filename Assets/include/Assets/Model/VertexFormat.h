#pragma once

#include "DllExport.h"

#include <Core/Containers/HashMap.h>

#include <Core/IO/OutputStream.h>

namespace Assets {

using VertexUsageName = uint32_t;

namespace VertexUsage {
    constexpr VertexUsageName POSITION = 0;
    constexpr VertexUsageName NORMAL   = 1;
    constexpr VertexUsageName COLOUR   = 2;
    constexpr VertexUsageName TEXTURE  = 3;
    constexpr VertexUsageName TEXTURE2 = 4;
}    // namespace VertexUsage

enum VertexPropertyFormat { FLOAT_32, FLOAT_64, INT_16, INT_32, UINT_16, UINT_32 };

struct ASSETS_API VertexProperty {
    VertexPropertyFormat format;
    VertexUsageName usage;
    uint8_t byteOffset;
    uint8_t elementCount;

    uint32_t byteCount() const;
};

struct ASSETS_API VertexFormat {
    Core::HashMap<VertexUsageName, VertexProperty> properties;
    uint32_t byteCount() const;
};
}    // namespace Assets

namespace Core::IO {
template <>
struct Serializer<Assets::VertexProperty> {
    static void serialize(Core::IO::OutputStream& stream, const Assets::VertexProperty& value) {
        stream.write(value.format);
        stream.write(value.usage);
        stream.write(value.byteOffset);
        stream.write(value.elementCount);
    }
};

template <>
struct Serializer<Assets::VertexFormat> {
    static void serialize(Core::IO::OutputStream& stream, const Assets::VertexFormat& value) {
        stream.write(value.properties);
    }
};
}    // namespace Core::IO