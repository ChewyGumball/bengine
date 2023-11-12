#pragma once

#include "core/io/serialization/compression/StreamingCompressionBuffer.h"

#include <zstd.h>

#include <streambuf>

namespace Core::IO::Compression {

class ZStdCompressionBuffer : public StreamingCompressionBuffer {
public:
    ZStdCompressionBuffer(Core::IO::OutputStream& stream, int32_t level);
    ~ZStdCompressionBuffer();

protected:
    std::streamsize xsputn(const std::byte* s, std::streamsize n) override;

private:
    Core::Array<std::byte> buffer;
    ZSTD_CCtx* zstdContext;
};

}    // namespace Core::IO::Compression
