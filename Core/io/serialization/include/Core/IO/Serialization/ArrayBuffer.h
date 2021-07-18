#pragma once

#include <Core/Containers/Array.h>

#include <ios>
#include <streambuf>

namespace Core::IO {
struct ArrayBuffer : public std::basic_streambuf<std::byte> {
    ArrayBuffer(size_t initialSize = 0);
    ArrayBuffer(Core::Array<std::byte>&& initialData);

    const Core::Array<std::byte>& buffer() const;

protected:
    virtual std::streamsize xsputn(const std::byte* s, std::streamsize n);
    virtual int_type overflow(int_type c = traits_type::eof());

    virtual std::streamsize showmanyc();
    virtual std::streamsize xsgetn(std::byte* s, std::streamsize n);

private:
    Core::Array<std::byte> data;
    void updatePointers(size_t inputOffset, size_t outputOffset);
    size_t currentOutputOffset();
    size_t currentInputOffset();
};
}    // namespace Core::IO