#include "Core/IO/Serialization/OutputStream.h"

#include <ostream>

namespace Core::IO {
OutputStream::OutputStream(std::basic_streambuf<std::byte>* stream)
  : stream(std::make_unique<std::basic_ostream<std::byte>>(stream)) {}

OutputStream::OutputStream(std::unique_ptr<std::basic_ostream<std::byte>>&& stream) : stream(std::move(stream)) {}

OutputStream::OutputStream(OutputStream&& other) : stream(std::move(other.stream)) {}

void OutputStream::write(std::span<const std::byte> data) {
    stream->write(data.data(), data.size());
}
void OutputStream::write(std::span<std::byte> data) {
    stream->write(data.data(), data.size());
}


void OutputStream::writeText(const std::string_view text) {
    write(std::as_bytes(std::span<const char>(text.data(), text.size())));
}

}    // namespace Core::IO