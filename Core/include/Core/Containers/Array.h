#pragma once

#include <array>
#include <vector>

#include "Core/IO/OutputStream.h"

namespace Core {

template <typename T>
using Array = std::vector<T>;

template <typename T, uint64_t SIZE>
using FixedArray = std::array<T, SIZE>;

}    // namespace Core

namespace Core::IO {
template <typename T>
struct Serializer<Core::Array<T>> {
    static void serialize(OutputStream& stream, const Core::Array<T>& values) {
        stream.write(values.size());
        stream.write(reinterpret_cast<const std::byte*>(values.data()), values.size() * sizeof(T));
    }
};

template <typename T, uint64_t SIZE>
struct Serializer<Core::FixedArray<T, SIZE>> {
    static void serialize(OutputStream& stream, const Core::FixedArray<T, SIZE>& values) {
        stream.write(values.size());
        stream.write(reinterpret_cast<const std::byte*>(values.data()), values.size() * sizeof(T));
    }
};
}    // namespace Core::IO