#pragma once


#include <Core/Containers/HashMap.h>

#include <Core/IO/InputStream.h>
#include <Core/IO/OutputStream.h>

namespace Assets {

using VertexUsageName = uint32_t;

namespace VertexUsage {
    constexpr VertexUsageName POSITION = 0;
    constexpr VertexUsageName NORMAL   = 1;
    constexpr VertexUsageName COLOUR   = 2;
    constexpr VertexUsageName TEXTURE  = 3;
    constexpr VertexUsageName TEXTURE2 = 4;

    constexpr std::string_view AsString(const VertexUsageName usage) {
        switch(usage) {
            case POSITION: return "Position";
            case NORMAL: return "Normal";
            case COLOUR: return "Colour";
            case TEXTURE: return "Texture";
            case TEXTURE2: return "Texture2";
            default: return "Unknown Property";
        }
    }
}    // namespace VertexUsage

using VertexPropertyFormatName = uint32_t;
namespace VertexPropertyFormat {
    constexpr VertexPropertyFormatName FLOAT_32 = 0;
    constexpr VertexPropertyFormatName FLOAT_64 = 1;
    constexpr VertexPropertyFormatName INT_16   = 2;
    constexpr VertexPropertyFormatName INT_32   = 3;
    constexpr VertexPropertyFormatName UINT_16  = 4;
    constexpr VertexPropertyFormatName UINT_32  = 5;

    constexpr std::string_view AsString(const VertexPropertyFormatName& format) {
        switch(format) {
            case FLOAT_32: return "32 bit float";
            case FLOAT_64: return "64 bit float";
            case INT_16: return "16 bit int";
            case INT_32: return "32 bit int";
            case UINT_16: return "16 bit uint";
            case UINT_32: return "32 bit uint";
            default: return "unknown format";
        }
    }
}    // namespace VertexPropertyFormat

struct VertexProperty {
    VertexPropertyFormatName format;
    VertexUsageName usage;
    uint8_t byteOffset;
    uint8_t elementCount;

    uint32_t byteCount() const;
};

struct VertexFormat {
    Core::HashMap<VertexUsageName, VertexProperty> properties;
    uint32_t byteCount() const;
};
}    // namespace Assets

namespace Core::IO {
template <>
struct Serializer<Assets::VertexFormat> {
    static void serialize(Core::IO::OutputStream& stream, const Assets::VertexFormat& value) {
        stream.write(value.properties);
    }
};

template <>
struct Deserializer<Assets::VertexFormat> {
    static Assets::VertexFormat deserialize(Core::IO::InputStream& stream) {
        return Assets::VertexFormat{
              .properties = stream.read<Core::HashMap<Assets::VertexUsageName, Assets::VertexProperty>>()};
    }
};
}    // namespace Core::IO