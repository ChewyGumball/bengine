#pragma once

#include <cassert>

namespace Renderer {
    template<typename T>
    struct BufferView {
        using ElementType = T;

        std::byte* const data;
        const uint32_t count;

        BufferView(std::byte* elementData, uint32_t elementCount)
            : data(elementData), count(elementCount) {}

        ElementType& operator[](std::size_t index) { 
            assert(index < count);

            std::size_t byteOffset = index * sizeof(ElementType);
            return *reinterpret_cast<ElementType*>(data + byteOffset); 
        }

        const ElementType& operator[](std::size_t index) const {
            assert(index < count);

            std::size_t byteOffset = index * sizeof(ElementType);
            return *reinterpret_cast<ElementType*>(data + byteOffset);
        }
    };
}