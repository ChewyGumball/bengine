#include "Core/IO/Serialization/ArrayBuffer.h"

namespace Core::IO {

ArrayBuffer::ArrayBuffer(size_t initialSize) : ArrayBuffer(Core::Array<std::byte>(initialSize)) {}
ArrayBuffer::ArrayBuffer(Core::Array<std::byte>&& initialData) : data(std::move(initialData)) {
    updatePointers(0, data.count());
}

const Core::Array<std::byte>& ArrayBuffer::buffer() const {
    return data;
}

Core::Array<std::byte> ArrayBuffer::takeBuffer() {
    return std::move(data);
}

std::streamsize ArrayBuffer::xsputn(const std::byte* s, std::streamsize n) {
    std::streamsize bytesWritten = 0;
    size_t outputOffset          = currentOutputOffset();

    while(outputOffset < data.count() && bytesWritten < n) {
        data[outputOffset++] = s[bytesWritten++];
    }

    while(bytesWritten < n) {
        data.emplace(s[bytesWritten++]);
        outputOffset++;
    }

    updatePointers(currentInputOffset(), outputOffset);
    return bytesWritten;
}

std::streamsize ArrayBuffer::xsgetn(std::byte* s, std::streamsize n) {
    std::streamsize bytesRead = 0;
    size_t inputOffset        = currentInputOffset();

    while(inputOffset < data.count() && bytesRead < n) {
        s[bytesRead++] = data[inputOffset++];
    }

    updatePointers(inputOffset, currentOutputOffset());
    return bytesRead;
}

void ArrayBuffer::updatePointers(size_t inputOffset, size_t outputOffset) {
    setg(data.begin(), data.begin(), data.end());
    setp(data.begin(), data.begin(), data.begin() + data.totalCapacity());
    gbump(static_cast<int>(inputOffset));
    pbump(static_cast<int>(outputOffset));
}

size_t ArrayBuffer::currentOutputOffset() {
    return pptr() - pbase();
}

size_t ArrayBuffer::currentInputOffset() {
    return gptr() - eback();
}
}    // namespace Core::IO