#include "core/io/serialization/compression/ZStdDecompressionBuffer.h"

#define ASSERT_ZSTD_NO_ERROR_IMPL(tempName, value)                \
    size_t tempName = (value);                                    \
    if(ZSTD_isError(tempName))                                    \
        [[unlikely]] {                                            \
            Core::Log::Critical(Core::internal::AssertLog,        \
                                "zstd error at {}:{} failed: {}", \
                                __FILE__,                         \
                                __LINE__,                         \
                                ZSTD_getErrorName(tempName));     \
            Core::Abort();                                        \
        }


#define ASSERT_ZSTD_NO_ERROR(value) \
    ASSERT_ZSTD_NO_ERROR_IMPL(CORE_ASSERT_CONCAT(return_if_error_status_, __COUNTER__), value)


namespace Core::IO::Compression {

ZStdDecompressionBuffer::ZStdDecompressionBuffer(Core::IO::InputStream& stream)
  : StreamingDecompressionBuffer(CompressionFormat::ZSTD, stream),
    buffer(ZSTD_DStreamInSize()),
    zstdContext(ZSTD_createDCtx()) {
    ASSERT_WITH_MESSAGE(zstdContext != nullptr, "Unable to create zstd decompression context!");

    setg(buffer.begin(), buffer.begin(), buffer.end());
}

ZStdDecompressionBuffer::~ZStdDecompressionBuffer() {
    ZSTD_freeDCtx(zstdContext);
}

std::streamsize ZStdDecompressionBuffer::xsgetn(std::byte* s, std::streamsize n) {
    ZSTD_outBuffer output{.dst = s, .size = size_t(n), .pos = 0};

    while(output.pos < output.size) {
        uint64_t inputSize = stream.readInto(buffer.rawData(), buffer.unusedCapacity());

        if(inputSize == 0) {
            break;
        }

        ZSTD_inBuffer input{.src = buffer.rawData(), .size = inputSize, .pos = 0};

        while(input.pos < input.size) {
            ASSERT_ZSTD_NO_ERROR(ZSTD_decompressStream(zstdContext, &output, &input));
        }
    }

    return output.pos;
}

}    // namespace Core::IO::Compression
