#include <Core/IO/Serialization/Compression.h>

#include <Core/IO/Serialization/Compression/ZStdCompression.h>
#include <Core/IO/Serialization/Compression/ZStdCompressionBuffer.h>
#include <Core/IO/Serialization/Compression/ZStdDecompressionBuffer.h>

namespace {}    // namespace

namespace Core::IO::Compression {

Core::StatusOr<Core::Array<std::byte>> Compress(Core::Span<const std::byte> bytes, CompressionFlags flags) {
    switch(flags.format) {
        case CompressionFormat::ZSTD: return ZStdCompress(bytes, flags);
        default: return Core::Status::Error("Decompression is not supported for compression format {}", flags.format);
    }
}

Core::StatusOr<Core::Array<std::byte>> Decompress(Core::Span<const std::byte> bytes,
                                                  std::optional<uint64_t> uncompressedBytes,
                                                  CompressionFlags flags) {
    switch(flags.format) {
        case CompressionFormat::ZSTD: return ZStdDecompress(bytes, uncompressedBytes, flags);
        default: return Core::Status::Error("Decompression is not supported for compression format {}", flags.format);
    }
}

Core::Status CompressToStream(Core::Span<const std::byte> bytes,
                              Core::IO::OutputStream& stream,
                              CompressionFlags flags) {
    ASSIGN_OR_RETURN(Core::Array<std::byte> compressedData, Compress(bytes, flags));
    stream.write(Core::ToSpan(compressedData));

    return Core::Status::Ok();
}

Core::StatusOr<Core::Array<std::byte>> DecompressFromStream(Core::IO::InputStream& stream,
                                                            std::optional<uint64_t> uncompressedBytes,
                                                            CompressionFlags flags) {
    uint64_t decompressedSize = 0;
    if(uncompressedBytes.has_value()) {
        decompressedSize = uncompressedBytes.value();
    } else if(flags.header == CompressionHeader::WITH_HEADER) {
        internal::CompressionHeader header = internal::ReadCompressionHeader(stream);

        decompressedSize = header.uncompressedBytes;
    } else {
        return Core::Status::Error(
              "Decompressing data without a header requires a decompressed byte count, but none was supplied.");
    }

    std::unique_ptr<StreamingDecompressionBuffer> decompressor = CreateStreamingDecompressionBuffer(stream, flags);
    Core::IO::InputStream decompressedStream(decompressor.get());

    Core::Array<std::byte> decompressedData(decompressedSize);
    Core::Span<std::byte> buffer = decompressedData.insertUninitialized(decompressedSize);

    uint64_t readSize = decompressedStream.readInto(buffer.rawData(), buffer.count());
    ASSERT_WITH_MESSAGE(readSize == decompressedSize,
                        "Invalid zstd decompression: decompressed to a different number of bytes than were compressed "
                        "(got {}, expected {})!",
                        readSize,
                        decompressedSize);

    return std::move(decompressedData);
}

std::unique_ptr<StreamingCompressionBuffer> CreateStreamingCompressionBuffer(Core::IO::OutputStream& stream,
                                                                             CompressionFlags flags) {
    switch(flags.format) {
        case CompressionFormat::ZSTD: {
            uint32_t level = ZStdLevelFromCompressionGoal(flags.goal);
            return std::make_unique<ZStdCompressionBuffer>(stream, level);
        }
        default:
            Core::AbortWithMessage("Streaming decompression is not supported for compression format {}", flags.format);
    }
}

std::unique_ptr<StreamingDecompressionBuffer> CreateStreamingDecompressionBuffer(Core::IO::InputStream& stream,
                                                                                 CompressionFlags flags) {
    switch(flags.format) {
        case CompressionFormat::ZSTD: return std::make_unique<ZStdDecompressionBuffer>(stream);
        default:
            Core::AbortWithMessage("Streaming decompression is not supported for compression format {}", flags.format);
    }
}

namespace internal {

void WriteCompressionHeader(const CompressionHeader& header, Core::IO::OutputStream& stream) {
    uint32_t formatAsInt = 0;
    switch(header.format) {
        case CompressionFormat::LZ4: formatAsInt = 1; break;
        case CompressionFormat::ZLIB: formatAsInt = 2; break;
        case CompressionFormat::ZSTD: formatAsInt = 3; break;
    }

    stream.write(formatAsInt);
    stream.write(header.uncompressedBytes);
}

CompressionHeader ReadCompressionHeader(Core::IO::InputStream& stream) {
    CompressionHeader header;

    uint32_t formatAsInt = stream.read<uint32_t>();
    switch(formatAsInt) {
        case 1: header.format = CompressionFormat::LZ4; break;
        case 2: header.format = CompressionFormat::ZLIB; break;
        case 3: header.format = CompressionFormat::ZSTD; break;
        default: Core::AbortWithMessage("Unknown compression format: {}", formatAsInt);
    }

    header.uncompressedBytes = stream.read<uint64_t>();

    return header;
}

}    // namespace internal

}    // namespace Core::IO::Compression

/*
#include <Core/IO/Serialization/ArrayBuffer.h>
#include <Core/IO/Serialization/MemoryBuffer.h>

#include <zlib/zlib.h>

namespace {
Core::Status ZLibResultToStatus(int ret, z_stream& stream) {
    switch(ret) {
        case Z_ERRNO: return Core::Status::Error("Could not read from/write to the zlib stream");
        case Z_STREAM_ERROR: return Core::Status::Error("Invalid zlib compression level");
        case Z_NEED_DICT:
        case Z_DATA_ERROR: return Core::Status::Error("Invalid or incomplete compressed input data");
        case Z_MEM_ERROR: return Core::Status::Error("zlib ran out of memory");
        case Z_VERSION_ERROR: return Core::Status::Error("zlib version mismatch");
        default: return Core::Status::Error("Unknown zlib error");
    }
}

Core::Status ZLibCheckDeflateStatus(int ret, z_stream& stream) {
    if(ret == Z_OK || ret == Z_STREAM_END) {
        return Core::Status::Ok();
    }

    deflateEnd(&stream);
    return ZLibResultToStatus(ret, stream);
}

Core::Status ZLibCheckInflateStatus(int ret, z_stream& stream) {
    if(ret == Z_OK) {
        return Core::Status::Ok();
    }

    inflateEnd(&stream);
    return ZLibResultToStatus(ret, stream);
}

constexpr size_t ZLIB_BUFFER_SIZE = 16384;    // 16kb

}    // namespace

namespace Core::IO {

Core::StatusOr<Core::Array<std::byte>> ZLibCompress(const Core::Array<std::byte>& data) {
    Core::IO::ArrayBuffer buffer;
    Core::IO::OutputStream stream(&buffer);

    RETURN_IF_ERROR(ZLibCompressToStream(data, stream));

    return buffer.takeBuffer();
}


Core::Status ZLibCompressToStream(const Core::Array<std::byte>& data, Core::IO::OutputStream& stream) {
    std::byte buffer[ZLIB_BUFFER_SIZE];

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    RETURN_IF_ERROR(ZLibCheckDeflateStatus(deflateInit(&strm, Z_DEFAULT_COMPRESSION), strm));

    strm.avail_in = data.count();
    strm.next_in  = reinterpret_cast<unsigned char*>(const_cast<std::byte*>(data.rawData()));

    do {
        strm.avail_out = ZLIB_BUFFER_SIZE;
        strm.next_out  = reinterpret_cast<unsigned char*>(&buffer[0]);

        RETURN_IF_ERROR(ZLibCheckDeflateStatus(deflate(&strm, Z_FINISH), strm));
        stream.write(std::span(buffer, ZLIB_BUFFER_SIZE - strm.avail_out));
    } while(strm.avail_out == 0);

    deflateEnd(&strm);

    return Core::Status::Ok();
}

Core::StatusOr<Core::Array<std::byte>> ZLibDecompress(const Core::Array<std::byte>& compressedData,
                                                      uint64_t uncompressedCount) {
    Core::IO::ReadOnlyMemoryBuffer buffer(compressedData);
    Core::IO::InputStream stream(&buffer);

    return ZLibDecompressFromStream(stream, uncompressedCount);
}

Core::StatusOr<Core::Array<std::byte>> ZLibDecompressFromStream(Core::IO::InputStream& stream,
                                                                uint64_t uncompressedCount) {
    std::byte buffer[ZLIB_BUFFER_SIZE];

    z_stream strm;
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;

    RETURN_IF_ERROR(ZLibCheckInflateStatus(inflateInit(&strm), strm));

    Core::Array<std::byte> decompressed;
    std::span<std::byte> decompressedBuffer = decompressed.insertUninitialized(uncompressedCount);
    strm.avail_out                          = decompressedBuffer.size();
    strm.next_out                           = reinterpret_cast<unsigned char*>(decompressedBuffer.data());

    int inflateStatus;
    do {
        strm.avail_in = stream.readInto(&buffer[0], ZLIB_BUFFER_SIZE);
        strm.next_in  = reinterpret_cast<unsigned char*>(&buffer[0]);

        do {
            inflateStatus = inflate(&strm, Z_NO_FLUSH);
            if(inflateStatus != Z_STREAM_END) {
                RETURN_IF_ERROR(ZLibCheckInflateStatus(inflateStatus, strm));
            }
        } while(strm.avail_in > 0 && inflateStatus != Z_STREAM_END);
    } while(inflateStatus != Z_STREAM_END);


    RETURN_IF_ERROR(stream.rewind(strm.avail_in));

    inflateEnd(&strm);

    return std::move(decompressed);
}


}    // namespace Core::IO
*/
