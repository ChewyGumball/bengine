#pragma once

#include "core/containers/Array.h"
#include "core/status/StatusOr.h"

#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"

#include <optional>

namespace Core::IO::Compression {

enum class CompressionFormat {
    ZLIB,
    ZSTD,
    LZ4,
};

enum class CompressionHeader {
    WITH_HEADER,
    WITHOUT_HEADER,
};

enum class CompressionGoal {
    SMALLEST_SIZE,
    BALANCED_SPEED_AND_SIZE,
    FASTEST_SPEED,
};

struct CompressionFlags {
    CompressionFormat format;
    CompressionHeader header;
    CompressionGoal goal;
};

constexpr CompressionFlags DEFAULT_BUFFER_FLAGS = CompressionFlags{
      .format = CompressionFormat::ZSTD,
      .header = CompressionHeader::WITHOUT_HEADER,
      .goal   = CompressionGoal::BALANCED_SPEED_AND_SIZE,
};

constexpr CompressionFlags DEFAULT_STREAM_FLAGS = CompressionFlags{
      .format = CompressionFormat::ZSTD,
      .header = CompressionHeader::WITH_HEADER,
      .goal   = CompressionGoal::BALANCED_SPEED_AND_SIZE,
};

Core::StatusOr<Core::Array<std::byte>> Compress(Core::Span<const std::byte> bytes,
                                                CompressionFlags flags = DEFAULT_BUFFER_FLAGS);

Core::StatusOr<Core::Array<std::byte>> Decompress(Core::Span<const std::byte> bytes,
                                                  std::optional<uint64_t> uncompressedBytes,
                                                  CompressionFlags flags = DEFAULT_BUFFER_FLAGS);

Core::Status CompressToStream(Core::Span<const std::byte> bytes,
                              Core::IO::OutputStream& stream,
                              CompressionFlags flags = DEFAULT_STREAM_FLAGS);

Core::StatusOr<Core::Array<std::byte>> DecompressFromStream(Core::IO::InputStream& stream,
                                                            std::optional<uint64_t> uncompressedBytes,
                                                            CompressionFlags flags = DEFAULT_STREAM_FLAGS);

std::unique_ptr<class StreamingCompressionBuffer>
CreateStreamingCompressionBuffer(Core::IO::OutputStream& stream, CompressionFlags flags = DEFAULT_STREAM_FLAGS);

std::unique_ptr<class StreamingDecompressionBuffer>
CreateStreamingDecompressionBuffer(Core::IO::InputStream& stream, CompressionFlags flags = DEFAULT_STREAM_FLAGS);

namespace internal {
struct CompressionHeader {
    CompressionFormat format;
    uint64_t uncompressedBytes;
};

constexpr uint64_t CompressionHeaderSize() {
    return sizeof(uint32_t) + sizeof(uint64_t);
}
void WriteCompressionHeader(const CompressionHeader& header, Core::IO::OutputStream& stream);
CompressionHeader ReadCompressionHeader(Core::IO::InputStream& stream);

}    // namespace internal

}    // namespace Core::IO::Compression
