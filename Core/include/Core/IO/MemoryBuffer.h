#pragma once

#include <streambuf>

#include "Core/DllExport.h"

#include "Core/Containers/Array.h"

namespace Core::IO {
struct CORE_API MemoryBuffer : public std::basic_streambuf<std::byte> {
    MemoryBuffer(std::byte* data, size_t size);
    MemoryBuffer(Core::Array<std::byte>& data);
};
}    // namespace Core