#pragma once

#include <Core/Containers/Array.h>

#include <streambuf>

namespace Core::IO {
struct MemoryBuffer : public std::basic_streambuf<std::byte> {
    MemoryBuffer(std::byte* data, size_t size);
    MemoryBuffer(Core::Array<std::byte>& data);
};
}    // namespace Core::IO