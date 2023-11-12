#pragma once

#include <Core/IO/Serialization/Compression.h>

namespace Core::IO::Compression {

uint32_t ZStdLevelFromCompressionGoal(CompressionGoal goal);

Core::StatusOr<Core::Array<std::byte>> ZStdCompress(Core::Span<const std::byte> bytes, CompressionFlags flags);
Core::StatusOr<Core::Array<std::byte>> ZStdDecompress(Core::Span<const std::byte> bytes,
                                                      std::optional<uint64_t> decompressedCount,
                                                      CompressionFlags flags);

}    // namespace Core::IO::Compression
