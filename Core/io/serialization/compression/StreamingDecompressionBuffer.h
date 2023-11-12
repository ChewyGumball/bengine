#pragma once

#include "core/io/serialization/compression/Compression.h"

#include <streambuf>

namespace Core::IO::Compression {

class StreamingDecompressionBuffer : public std::basic_streambuf<std::byte> {
public:
    const CompressionFormat compressionFormat;

    StreamingDecompressionBuffer(CompressionFormat compressionFormat, Core::IO::InputStream& stream)
      : compressionFormat(compressionFormat), stream(stream) {}

    virtual ~StreamingDecompressionBuffer() = default;


protected:
    Core::IO::InputStream& stream;
};

}    // namespace Core::IO::Compression
