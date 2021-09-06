#include <Core/IO/Serialization/Compression.h>

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
