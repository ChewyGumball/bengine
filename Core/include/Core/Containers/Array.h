#pragma once

#include <array>
#include <assert.h>
#include <type_traits>
#include <vector>

#include "Core/IO/InputStream.h"
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
        if constexpr(std::is_trivial_v<T>) {
            stream.write(reinterpret_cast<const std::byte*>(values.data()), values.size() * sizeof(T));
        } else {
            for(const T& value : values) {
                Serializer<T>::serialize(stream, value);
            }
        }
    }
};

template <typename T, uint64_t SIZE>
struct Serializer<Core::FixedArray<T, SIZE>> {
    static void serialize(OutputStream& stream, const Core::FixedArray<T, SIZE>& values) {
        stream.write(values.size());
        if constexpr(std::is_trivial_v<T>) {
            stream.write(reinterpret_cast<const std::byte*>(values.data()), values.size() * sizeof(T));
        } else {
            for(const T& value : values) {
                Serializer<T>::serialize(stream, value);
            }
        }
    }
};

template <typename T>
struct Deserializer<Core::Array<T>> {
    static Core::Array<T> deserialize(InputStream& stream) {
        size_t count = stream.read<Core::Array<T>::size_type>();
        if constexpr(std::is_trivial_v<T>) {
            Core::Array<T> value(count);
            stream.readInto(reinterpret_cast<std::byte*>(value.data()), value.size() * sizeof(T));
            return value;
        } else {
            Core::Array<T> value;
            for(size_t i = 0; i < count; i++) {
                value.emplace_back(Deserializer<T>::deserialize(stream));
            }
            return value;
        }
    }
};

template <typename T, uint64_t SIZE>
struct Deserializer<Core::FixedArray<T, SIZE>> {
    static Core::FixedArray<T, SIZE> deserialize(InputStream& stream) {
        size_t count = stream.read<Core::FixedArray<T, SIZE>::size_type>();
        assert(count == SIZE);

        Core::FixedArray<T, SIZE> value;

        if constexpr(std::is_trivial_v<T>) {
            stream.readInto(reinterpret_cast<std::byte*>(value.data()), value.size() * sizeof(T));
        } else {
            for(size_t i = 0; i < count; i++) {
                value[i] = Deserializer<T>::deserialize(stream);
            }
        }

        return value;
    }
};

}    // namespace Core::IO
