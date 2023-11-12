#include "core/io/serialization/OutputStream.h"

#include <ostream>

namespace Core::IO {
OutputStream::OutputStream(std::basic_streambuf<std::byte>* stream)
  : stream(std::make_unique<std::basic_ostream<std::byte>>(stream)) {}

OutputStream::OutputStream(std::unique_ptr<std::basic_ostream<std::byte>>&& stream) : stream(std::move(stream)) {}

OutputStream::OutputStream(OutputStream&& other) : stream(std::move(other.stream)) {}

void OutputStream::write(Core::Span<const std::byte> data) {
    stream->write(data.rawData(), data.count());
}

void OutputStream::write(Core::Span<std::byte> data) {
    stream->write(data.rawData(), data.count());
}

void OutputStream::writeText(const std::string_view text) {
    write(Core::AsBytes(Core::ToSpan(text)));
}

}    // namespace Core::IO