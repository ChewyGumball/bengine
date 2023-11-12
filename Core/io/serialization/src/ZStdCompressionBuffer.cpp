#include <Core/IO/Serialization/Compression/ZStdCompressionBuffer.h>

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

ZStdCompressionBuffer::ZStdCompressionBuffer(Core::IO::OutputStream& stream, int32_t level)
  : StreamingCompressionBuffer(CompressionFormat::ZSTD, stream),
    buffer(ZSTD_CStreamOutSize()),
    zstdContext(ZSTD_createCCtx()) {
    ASSERT_WITH_MESSAGE(zstdContext != nullptr, "Unable to create zstd compression context!");

    ASSERT_ZSTD_NO_ERROR(ZSTD_CCtx_setParameter(zstdContext, ZSTD_c_compressionLevel, level));

    setp(buffer.begin(), buffer.begin(), buffer.end());
}

ZStdCompressionBuffer::~ZStdCompressionBuffer() {
    ZSTD_inBuffer input{.src = nullptr, .size = 0, .pos = 0};

    uint32_t remaining;
    do {
        ZSTD_outBuffer output{.dst = buffer.rawData(), .size = buffer.unusedCapacity(), .pos = 0};
        remaining = ZSTD_compressStream2(zstdContext, &output, &input, ZSTD_e_end);
        ASSERT_ZSTD_NO_ERROR(remaining);

        std::span<std::byte> newData(buffer.rawData(), output.pos);
        stream.write(newData);

        // We don't need to clear the output buffer because we just overwrite the data the next time
        // we compress to it. It doesn't actually keep track of the number of elements.
    } while(remaining != 0);

    ZSTD_freeCCtx(zstdContext);
}

std::streamsize ZStdCompressionBuffer::xsputn(const std::byte* s, std::streamsize n) {
    ZSTD_inBuffer input{.src = s, .size = size_t(n), .pos = 0};

    while(input.pos != input.size) {
        ZSTD_outBuffer output{.dst = buffer.rawData(), .size = buffer.count(), .pos = 0};
        ASSERT_ZSTD_NO_ERROR(ZSTD_compressStream2(zstdContext, &output, &input, ZSTD_e_continue));

        std::span<std::byte> newData(buffer.rawData(), output.pos);
        stream.write(newData);

        // We don't need to clear the output buffer because we just overwrite the data the next time
        // we compress to it. It doesn't actually keep track of the number of elements.
    }

    return input.pos;
}

}    // namespace Core::IO::Compression
