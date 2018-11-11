#include "Core/IO/MemoryBuffer.h"

namespace Core::IO {

MemoryBuffer::MemoryBuffer(std::byte* data, size_t size) {
    setg(data, data, data + size);
    setp(data, data + size);
}
MemoryBuffer::MemoryBuffer(Core::Array<std::byte>& data) : MemoryBuffer(data.data(), data.size()) {}
}    // namespace Core::IO