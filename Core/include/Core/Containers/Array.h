#pragma once

#include <Core/Assert.h>

#include "Core/IO/InputStream.h"
#include "Core/IO/OutputStream.h"

#include <algorithm>
#include <array>
#include <span>
#include <type_traits>
#include <vector>

namespace Core {

template <typename T, uint64_t SIZE>
using FixedArray = std::array<T, SIZE>;

template <typename T>
class Array {
public:
    constexpr static uint64_t MinimumCapacity = 4;
    constexpr static uint64_t ElementSize     = sizeof(T);
    constexpr static float GrowthFactor       = 1.5f;

    using ElementType = T;

    Array(uint64_t initialCapacity = 4);
    Array(const T& original, uint64_t repeatCount);
    Array(std::initializer_list<T> initializerList);
    Array(const Array<T>& other);
    Array(Array<T>&& other);

    Array& operator=(const Array<T>& other);
    Array& operator=(Array<T>&& other);

    ~Array();

    [[nodiscard]] T& operator[](uint64_t i);
    [[nodiscard]] const T& operator[](uint64_t i) const;

    template <typename... ARGS>
    T& emplaceAt(uint64_t index, ARGS&&... args);

    template <typename... ARGS>
    T& emplace(ARGS&&... args);

    T& insertAt(uint64_t index, const T& elementToInsert);
    T& insertAt(uint64_t index, T&& elementToInsert);

    T& insert(const T& elementToInsert);
    T& insert(T&& elementToInsert);

    template <typename U, std::size_t EXTENT>
    std::span<T, EXTENT> insertAll(std::span<U, EXTENT> elements);

    [[nodiscard]] std::span<T> insertUninitialized(uint64_t newElementCount);

    void eraseAt(uint64_t index);

    void clear();

    [[nodiscard]] uint64_t count() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] T* rawData();
    [[nodiscard]] const T* rawData() const;
    [[nodiscard]] T* begin();
    [[nodiscard]] const T* begin() const;
    [[nodiscard]] T* end();
    [[nodiscard]] const T* end() const;

    void ensureCapacity(uint64_t requiredCapacity);

protected:
    void destructElements(uint64_t startIndex, uint64_t elementsToDestruct);
    void moveElements(uint64_t sourceIndex, uint64_t destinationIndex, uint64_t elementCount);

    uint64_t capacity;
    uint64_t elementCount = 0;
    T* data               = nullptr;
};


template <typename T>
std::span<T> ToSpan(Core::Array<T>& array) {
    return std::span<T>(array.rawData(), array.count());
}

template <typename T>
std::span<const T> ToSpan(const Core::Array<T>& array) {
    return std::span<const T>(array.rawData(), array.count());
}

template <typename T, uint64_t SIZE>
std::span<T, SIZE> ToSpan(Core::FixedArray<T, SIZE>& array) {
    return std::span<T>(array.data(), SIZE);
}

template <typename T, uint64_t SIZE>
std::span<const T, SIZE> ToSpan(const Core::FixedArray<T, SIZE>& array) {
    return std::span<T>(array.data(), SIZE);
}

}    // namespace Core

namespace Core::IO {

template <typename T>
struct Serializer<Core::Array<T>> {
    static void serialize(OutputStream& stream, const Core::Array<T>& values) {
        stream.write(values.count());
        if constexpr(std::is_trivially_copyable_v<T>) {
            stream.write(std::as_bytes(Core::ToSpan(values)));
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
            stream.write(std::as_bytes(Core::ToSpan(values)));
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
        uint64_t elementCount = stream.read<uint64_t>();
        Core::Array<T> value;

        if constexpr(std::is_trivially_copyable_v<T>) {
            stream.readInto(value.insertUninitialized(elementCount));
        } else {
            for(size_t i = 0; i < elementCount; i++) {
                value.emplace(Deserializer<T>::deserialize(stream));
            }
        }
        return value;
    }
};

template <typename T, uint64_t SIZE>
struct Deserializer<Core::FixedArray<T, SIZE>> {
    static Core::FixedArray<T, SIZE> deserialize(InputStream& stream) {
        size_t elementCount = stream.read<Core::FixedArray<T, SIZE>::size_type>();
        ASSERT(elementCount == SIZE);

        Core::FixedArray<T, SIZE> value;

        if constexpr(std::is_trivial_v<T>) {
            stream.readInto(reinterpret_cast<std::byte*>(value.data()), value.size() * sizeof(T));
        } else {
            for(size_t i = 0; i < elementCount; i++) {
                value[i] = Deserializer<T>::deserialize(stream);
            }
        }

        return value;
    }
};

}    // namespace Core::IO

#include <Core/Containers/internal/Array.inl>
