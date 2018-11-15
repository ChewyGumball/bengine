#pragma once

#include <assert.h>
#include <cstddef>
#include <stdint.h>

#include "Core/Containers/Array.h"

#include "Core/DllExport.h"

namespace Core::IO {

struct BufferView {
    std::byte* const data;
    const uint64_t size;

    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, T&> read(uint64_t position) {
        assert(position + sizeof(T) <= size);
        return *reinterpret_cast<T*>(data + position);
    }
};

struct CORE_API BufferViewWindow {
    const BufferView view;
    const uint64_t window;
    uint64_t windowStart;

    BufferViewWindow(BufferView bufferView, uint64_t windowSize);
    BufferViewWindow(Core::Array<std::byte>& buffer, uint64_t windowSize);


    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, T&> read(uint64_t position) const {
        assert(position + sizeof(T) <= window);
        return *reinterpret_cast<T*>(view.data + windowStart + position);
    }

    const std::byte* operator*() const;
    operator bool() const;
};

CORE_API BufferViewWindow& operator++(BufferViewWindow& view);
bool CORE_API operator==(const BufferViewWindow& a, const BufferViewWindow& b);
bool CORE_API operator!=(const BufferViewWindow& a, const BufferViewWindow& b);
}    // namespace Core::IO