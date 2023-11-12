#pragma once

#include <Core/Containers/Span.h>

#include <streambuf>

namespace Core::IO {
struct MemoryBuffer : public std::basic_streambuf<std::byte> {
    MemoryBuffer(std::byte* data, size_t size);
    MemoryBuffer(Core::Span<std::byte> data);
};

struct ReadOnlyMemoryBuffer : public std::basic_streambuf<std::byte> {
    ReadOnlyMemoryBuffer(const std::byte* data, size_t size);
    ReadOnlyMemoryBuffer(Core::Span<const std::byte> data);
};

}    // namespace Core::IO