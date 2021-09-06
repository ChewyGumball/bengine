#pragma once

#include <Core/Containers/Array.h>
#include <Core/Status/StatusOr.h>

#include <Core/IO/Serialization/InputStream.h>
#include <Core/IO/Serialization/OutputStream.h>

namespace Core::IO {

Core::StatusOr<Core::Array<std::byte>> ZLibCompress(const Core::Array<std::byte>& data);
Core::Status ZLibCompressToStream(const Core::Array<std::byte>& data, Core::IO::OutputStream& stream);

Core::StatusOr<Core::Array<std::byte>> ZLibDecompress(const Core::Array<std::byte>& compressedData,
                                                      uint64_t uncompressedCount);
Core::StatusOr<Core::Array<std::byte>> ZLibDecompressFromStream(Core::IO::InputStream& stream,
                                                                uint64_t uncompressedCount);

}    // namespace Core::IO
