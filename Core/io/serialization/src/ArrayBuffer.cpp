#include "Core/IO/Serialization/ArrayBuffer.h"

namespace Core::IO {

ArrayBuffer::ArrayBuffer(size_t initialSize) : ArrayBuffer(Core::Array<std::byte>(std::byte{0}, initialSize)) {}
ArrayBuffer::ArrayBuffer(Core::Array<std::byte>&& initialData) : data(std::move(initialData)) {
    updatePointers(0, 0);
}

const Core::Array<std::byte>& ArrayBuffer::buffer() const {
    return data;
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
std::basic_streambuf<std::byte>::int_type ArrayBuffer::overflow(std::basic_streambuf<std::byte>::int_type c) {
    if(c != traits_type::eof()) {
        data.emplace(static_cast<std::byte>(c));
        updatePointers(currentInputOffset(), data.count());
    }

    return c;
}

std::streamsize ArrayBuffer::showmanyc() {
    return data.count() - currentInputOffset();
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
    setp(data.begin(), data.begin(), data.end());
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