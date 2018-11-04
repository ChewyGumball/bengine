#pragma once

#include <istream>
#include <type_traits>

#include "Core/Containers/Array.h"

namespace Core {
class InputStream {
private:
    std::istream* stream;

public:
    InputStream(std::istream* stream) : stream(stream) {}

    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, T> read() {
        T value;
        stream.read(reinterpret_cast<std::byte*>(&value), sizeof(T));
        return value;
    }

    template <typename T>
    typename std::enable_if_t<std::is_trivially_constructible_v<T>, Core::Array<T>> read(size_t count) {
        Core::Array<T> values(count);
        stream.read(reinterpret_cast<std::byte*>(values.data()), count * sizeof(T));
        return values;
    }
};
}    // namespace Core