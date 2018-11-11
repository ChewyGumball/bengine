#pragma once

#include <assert.h>
#include <cstddef>
#include <stdint.h>

#include "Core/Containers/Array.h"

#include "Core/DllExport.h"

namespace Core::IO {

struct BufferView {
    const std::byte* const data;
    const uint64_t size;

    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, T&> read(uint64_t position) {
        assert(position + sizeof(T) <= size);
        return *reinterpret_cast<T*>(data + position);
    }
};

struct CORE_API WindowedBufferView {
    std::byte* data;
    uint64_t size;
    const uint64_t windowSize;

    WindowedBufferView(std::byte* buffer, uint64_t bufferSize, uint64_t window);
    WindowedBufferView(Core::Array<std::byte>& buffer, uint64_t window);


    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, T&> read(uint64_t position) const {
        assert(position + sizeof(T) <= size);
        return *reinterpret_cast<T*>(data + position);
    }

    const std::byte* operator*() const;
    operator bool() const;
};

CORE_API WindowedBufferView& operator++(WindowedBufferView& view);
bool CORE_API operator==(const WindowedBufferView& a, const WindowedBufferView& b);
bool CORE_API operator!=(const WindowedBufferView& a, const WindowedBufferView& b);
}    // namespace Core::IO