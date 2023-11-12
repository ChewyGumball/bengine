#include <Core/IO/Serialization/Compression/ZStdCompression.h>

#include <Core/IO/Serialization/ArrayBuffer.h>
#include <Core/IO/Serialization/MemoryBuffer.h>

#include <zstd.h>


namespace Core::IO::Compression {

uint32_t ZStdLevelFromCompressionGoal(CompressionGoal goal) {
    switch(goal) {
        case CompressionGoal::SMALLEST_SIZE: return 22;
        case CompressionGoal::BALANCED_SPEED_AND_SIZE: return 3;
        case CompressionGoal::FASTEST_SPEED: return 1;
        default: Core::AbortWithMessage("Unknown compression goal: {}", goal);
    }
}

Core::StatusOr<Core::Array<std::byte>> ZStdCompress(Core::Span<const std::byte> bytes, CompressionFlags flags) {
    uint64_t expectedCompressedSize = ZSTD_compressBound(bytes.count());
    uint64_t bufferSizeGuess        = internal::CompressionHeaderSize() + expectedCompressedSize;

    Core::IO::ArrayBuffer buffer(bufferSizeGuess);
    Core::IO::OutputStream stream(&buffer);

    if(flags.header == CompressionHeader::WITH_HEADER) {
        internal::CompressionHeader header{
              .format            = CompressionFormat::ZSTD,
              .uncompressedBytes = bytes.count(),
        };
        internal::WriteCompressionHeader(header, stream);
    }

    uint32_t level                        = ZStdLevelFromCompressionGoal(flags.goal);
    Core::Array<std::byte> compressedData = buffer.takeBuffer();

    uint64_t maximumCompressedSize   = compressedData.unusedCapacity();
    Core::Span<std::byte> compressed = compressedData.insertUninitialized(maximumCompressedSize);

    uint32_t compressedSize =
          ZSTD_compress(compressed.rawData(), compressed.count(), bytes.rawData(), bytes.count(), level);

    if(ZSTD_isError(compressedSize)) {
        return Core::Status::Error("Error during zstd compression: {}", ZSTD_getErrorName(compressedSize));
    }

    uint64_t garbageElements       = maximumCompressedSize - compressedSize;
    uint64_t garbageElementsOffset = compressedData.count() - garbageElements;

    compressedData.eraseAt(garbageElementsOffset, garbageElements);
    return std::move(compressedData);
}


Core::StatusOr<Core::Array<std::byte>> ZStdDecompress(Core::Span<const std::byte> bytes,
                                                      std::optional<uint64_t> decompressedCount,
                                                      CompressionFlags flags) {
    uint64_t uncompressedSize = 0;
    if(flags.header == CompressionHeader::WITH_HEADER) {
        Core::IO::ReadOnlyMemoryBuffer buffer(bytes);
        Core::IO::InputStream stream(&buffer);

        internal::CompressionHeader header = internal::ReadCompressionHeader(stream);

        bytes.truncateFront(internal::CompressionHeaderSize());
        uncompressedSize = header.uncompressedBytes;

        if(decompressedCount.has_value() && decompressedCount.value() != uncompressedSize) {
            return Core::Status::Error(
                  "Given decompressed size ({}) does not match compression header's decompressed size ({})!",
                  decompressedCount.value(),
                  uncompressedSize);
        }
    } else if(decompressedCount.has_value()) {
        uncompressedSize = decompressedCount.value();
    } else {
        return Core::Status::Error(
              "Decompressing zstd data without a header requires a decompressed byte count, but none was supplied.");
    }

    Core::Array<std::byte> uncompressedData(uncompressedSize);
    Core::Span<std::byte> buffer = uncompressedData.insertUninitialized(uncompressedSize);

    uint64_t decompressedSize = ZSTD_decompress(buffer.rawData(), buffer.count(), bytes.rawData(), bytes.count());

    if(ZSTD_isError(decompressedSize)) {
        return Core::Status::Error("Error during zstd decompression: {}", ZSTD_getErrorName(decompressedSize));
    }

    ASSERT_WITH_MESSAGE(uncompressedSize == decompressedSize,
                        "Invalid zstd decompression: decompressed to a different number of bytes than were compressed "
                        "(got {}, expected {})!",
                        decompressedSize,
                        uncompressedSize);

    return std::move(uncompressedData);
}

}    // namespace Core::IO::Compression
