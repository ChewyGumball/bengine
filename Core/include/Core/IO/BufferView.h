#pragma once

#include <Core/Assert.h>
#include <Core/Containers/Array.h>

#include <cstddef>
#include <stdint.h>

namespace Core::IO {

struct BufferView {
    std::byte* const data;
    const uint64_t size;

    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, T&> read(uint64_t position) {
        ASSERT(position + sizeof(T) <= size);
        return *reinterpret_cast<T*>(data + position);
    }
};

struct BufferViewWindow {
    const BufferView view;
    const uint64_t window;
    uint64_t windowStart;

    BufferViewWindow(BufferView bufferView, uint64_t windowSize);
    BufferViewWindow(Core::Array<std::byte>& buffer, uint64_t windowSize);


    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, T&> read(uint64_t position) const {
        ASSERT(position + sizeof(T) <= window);
        return *reinterpret_cast<T*>(view.data + windowStart + position);
    }

    const std::byte* operator*() const;
    operator bool() const;
};

BufferViewWindow& operator++(BufferViewWindow& view);
bool operator==(const BufferViewWindow& a, const BufferViewWindow& b);
bool operator!=(const BufferViewWindow& a, const BufferViewWindow& b);
}    // namespace Core::IO