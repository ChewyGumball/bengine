#include "Core/IO/Serialization/InputStream.h"

namespace Core::IO {
InputStream::InputStream(std::basic_streambuf<std::byte>* stream)
  : stream(std::make_unique<class std::basic_istream<std::byte>>(stream)) {}

InputStream::InputStream(std::unique_ptr<class std::basic_istream<std::byte>>&& stream) : stream(std::move(stream)) {}

InputStream::InputStream(InputStream&& other) : stream(std::move(other.stream)) {}

uint64_t InputStream::readInto(std::byte* buffer, uint64_t size) {
    stream->read(buffer, size);
    return stream->gcount();
}

Core::Status InputStream::rewind(uint64_t byteCount) {
    for(uint64_t i = 0; i < byteCount; i++) {
        if(!stream->unget()) {
            return Core::Status::Error(
                  "Tried to rewind the stream by {} bytes, but failed after {} bytes", byteCount, i);
        }
    }

    return Core::Status::Ok();
}
}    // namespace Core::IO