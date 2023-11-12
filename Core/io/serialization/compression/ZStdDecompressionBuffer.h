#pragma once

#include "core/io/serialization/compression/StreamingDecompressionBuffer.h"

#include <zstd.h>

#include <streambuf>

namespace Core::IO::Compression {

class ZStdDecompressionBuffer : public StreamingDecompressionBuffer {
public:
    ZStdDecompressionBuffer(Core::IO::InputStream& stream);
    ~ZStdDecompressionBuffer();

protected:
    std::streamsize xsgetn(std::byte* s, std::streamsize n) override;

private:
    Core::Array<std::byte> buffer;
    ZSTD_DCtx* zstdContext;
};

}    // namespace Core::IO::Compression
