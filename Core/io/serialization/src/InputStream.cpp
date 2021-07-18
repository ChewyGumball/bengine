#include "Core/IO/Serialization/InputStream.h"

namespace Core::IO {
InputStream::InputStream(std::basic_streambuf<std::byte>* stream)
  : stream(std::make_unique<class std::basic_istream<std::byte>>(stream)) {}

InputStream::InputStream(std::unique_ptr<class std::basic_istream<std::byte>>&& stream) : stream(std::move(stream)) {}

InputStream::InputStream(InputStream&& other) : stream(std::move(other.stream)) {}

void InputStream::readInto(std::byte* buffer, uint64_t size) {
    stream->read(buffer, size);
}

}    // namespace Core::IO