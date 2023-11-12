#pragma once

#include "core/io/serialization/compression/Compression.h"

#include <streambuf>

namespace Core::IO::Compression {

class StreamingCompressionBuffer : public std::basic_streambuf<std::byte> {
public:
    const CompressionFormat compressionFormat;

    StreamingCompressionBuffer(CompressionFormat compressionFormat, Core::IO::OutputStream& stream)
      : compressionFormat(compressionFormat), stream(stream) {}

    virtual ~StreamingCompressionBuffer() = default;

protected:
    Core::IO::OutputStream& stream;
};

}    // namespace Core::IO::Compression
