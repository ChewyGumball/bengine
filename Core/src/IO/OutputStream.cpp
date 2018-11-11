#include "Core/IO/OutputStream.h"

#include <ostream>

namespace Core::IO {
OutputStream::OutputStream(std::basic_streambuf<std::byte>* stream)
  : stream(std::make_unique<std::basic_ostream<std::byte>>(stream)) {}

OutputStream::OutputStream(std::unique_ptr<std::basic_ostream<std::byte>>&& stream) : stream(std::move(stream)) {}

OutputStream::OutputStream(OutputStream&& other) : stream(std::move(other.stream)) {}

void OutputStream::write(const std::byte* data, uint64_t size) {
    stream->write(data, size);
}
}    // namespace Core::IO