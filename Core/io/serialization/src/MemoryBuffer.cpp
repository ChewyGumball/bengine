#include "Core/IO/Serialization/MemoryBuffer.h"

namespace Core::IO {

MemoryBuffer::MemoryBuffer(std::byte* data, size_t size) {
    setg(data, data, data + size);
    setp(data, data + size);
}
MemoryBuffer::MemoryBuffer(Core::Array<std::byte>& data) : MemoryBuffer(data.rawData(), data.count()) {}

ReadOnlyMemoryBuffer::ReadOnlyMemoryBuffer(const std::byte* data, size_t size) {
    std::byte* badbadMemory = const_cast<std::byte*>(data);

    setg(badbadMemory, badbadMemory, badbadMemory + size);
    setp(badbadMemory + size, badbadMemory + size);    // The beginning and end are set to be the same because we are
                                                       // not allowed to write to this buffer (it is const data)
}
ReadOnlyMemoryBuffer::ReadOnlyMemoryBuffer(const Core::Array<std::byte>& data)
  : ReadOnlyMemoryBuffer(data.rawData(), data.count()) {}
}    // namespace Core::IO