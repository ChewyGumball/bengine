#pragma once

#include <Core/Containers/Array.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/IO/Serialization/OutputStream.h>

#include <Core/IO/Serialization/Compression.h>

namespace Assets {

using TextureFormatType = uint32_t;
namespace TextureFormat {
constexpr TextureFormatType RGBA_8_FLOAT = 0;
constexpr TextureFormatType RGBA_8_UINT  = 1;
constexpr TextureFormatType RGBA_8_INT   = 2;
}    // namespace TextureFormat

struct Texture {
    uint32_t height;
    uint32_t width;

    TextureFormatType format;

    Core::Array<std::byte> data;
};

}    // namespace Assets

namespace Core::IO {

template <>
struct Serializer<Assets::Texture> {
    static void serialize(OutputStream& stream, const Assets::Texture& value) {
        stream.write(value.height);
        stream.write(value.width);
        stream.write(value.format);

        stream.write(value.data.count());
        ASSERT_IS_OK(ZLibCompressToStream(value.data, stream));
    }
};

template <>
struct Deserializer<Assets::Texture> {
    static Assets::Texture deserialize(InputStream& stream) {
        auto height = stream.read<uint32_t>();
        auto width  = stream.read<uint32_t>();
        auto format = stream.read<Assets::TextureFormatType>();

        auto uncompressedCount = stream.read<uint64_t>();
        ASSIGN_OR_ASSERT(auto data, ZLibDecompressFromStream(stream, uncompressedCount));


        return Assets::Texture{.height = height, .width = width, .format = format, .data = std::move(data)};
    }
};
}    // namespace Core::IO
