#pragma once

#include "core/io/serialization/compression/StreamingCompressionBuffer.h"

#include <streambuf>

namespace Core::IO::Compression {

class ZLibCompressionBuffer : public StreamingCompressionBuffer {
public:
    ZLibCompressionBuffer(CompressionFormat compressionType, Core::IO::OutputStream& stream);
    ~ZLibCompressionBuffer();
};

}    // namespace Core::IO::Compression
