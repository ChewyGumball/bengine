#pragma once

#include <Core/Containers/Array.h>

#include <ios>
#include <streambuf>

namespace Core::IO {
struct ArrayBuffer : public std::basic_streambuf<std::byte> {
    ArrayBuffer(size_t initialSize = 0);
    ArrayBuffer(Core::Array<std::byte>&& initialData);

    const Core::Array<std::byte>& buffer() const;

    [[nodiscard]] Core::Array<std::byte> takeBuffer();

protected:
    std::streamsize xsputn(const std::byte* s, std::streamsize n) override;
    std::streamsize xsgetn(std::byte* s, std::streamsize n) override;

private:
    Core::Array<std::byte> data;
    void updatePointers(size_t inputOffset, size_t outputOffset);
    size_t currentOutputOffset();
    size_t currentInputOffset();
};
}    // namespace Core::IO