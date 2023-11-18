#pragma once

#include "core/Types.h"
#include "core/assert/Assert.h"

#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"

#include <algorithm>
#include <array>
#include <span>
#include <type_traits>
#include <vector>

namespace Core {

template <typename T, u64 SIZE>
using FixedArray = std::array<T, SIZE>;

template <typename T>
class Array {
public:
    constexpr static u64 MinimumCapacity = 4;
    constexpr static u64 ElementSize     = sizeof(T);
    constexpr static f32 GrowthFactor    = 1.5f;

    using ElementType = T;

    explicit Array(u64 initialCapacity = 4);

    explicit Array(Core::Span<T> elementsToCopy);

    Array(const T& original, u64 repeatCount);
    Array(std::initializer_list<T> initializerList);
    Array(const Array<T>& other);
    Array(Array<T>&& other);

    Array<T>& operator=(const Array<T>& other);
    Array<T>& operator=(Array<T>&& other);

    ~Array();

    [[nodiscard]] T& operator[](u64 i);
    [[nodiscard]] const T& operator[](u64 i) const;

    template <typename... ARGS>
    T& emplaceAt(u64 index, ARGS&&... args);

    template <typename... ARGS>
    T& emplace(ARGS&&... args);

    T& insertAt(u64 index, const T& elementToInsert);
    T& insertAt(u64 index, T&& elementToInsert);

    T& insert(const T& elementToInsert);
    T& insert(T&& elementToInsert);

    template <typename U>
    Core::Span<T> insertAll(Core::Span<U> elements);

    [[nodiscard]] Core::Span<T> insertUninitialized(u64 newElementCount);

    void eraseAt(u64 index, u64 elementsToErase = 1);

    void clear();

    [[nodiscard]] u64 count() const;
    [[nodiscard]] u64 totalCapacity() const;
    [[nodiscard]] u64 unusedCapacity() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] T* rawData();
    [[nodiscard]] const T* rawData() const;
    [[nodiscard]] T* begin();
    [[nodiscard]] const T* begin() const;
    [[nodiscard]] T* end();
    [[nodiscard]] const T* end() const;

    void ensureCapacity(u64 requiredCapacity);

protected:
    void destructAllElements();

    static void CopyElementMemory(T* destination, const T* source, u64 elementCount);
    static void MoveElementMemory(T* destination, const T* source, u64 elementCount);

    void shiftElementsLeft(u64 startIndex, u64 distance);
    void shiftElementsRight(u64 startIndex, u64 distance);

    u64 capacity;
    u64 elementCount = 0;
    T* data          = nullptr;
};

template <typename T>
Core::Span<T> ToSpan(Core::Array<T>& array) {
    return Core::Span<T>(array.rawData(), array.count());
}

template <typename T>
Core::Span<const T> ToSpan(const Core::Array<T>& array) {
    return Core::Span<const T>(array.rawData(), array.count());
}

template <typename T, u64 SIZE>
Core::Span<T> ToSpan(Core::FixedArray<T, SIZE>& array) {
    return Core::Span<T>(array.data(), SIZE);
}

template <typename T, u64 SIZE>
Core::Span<const T> ToSpan(const Core::FixedArray<T, SIZE>& array) {
    return Core::Span<T>(array.data(), SIZE);
}

template <typename T>
Core::Span<std::byte> AsBytes(Core::Array<T>& array) requires std::is_trivially_copyable_v<T> {
    return Core::AsWritableBytes(ToSpan(array));
}

template <typename T>
Core::Span<const std::byte> AsBytes(const Core::Array<T>& array) requires std::is_trivially_copyable_v<T> {
    return Core::AsBytes(ToSpan(array));
}

template <typename T, u64 SIZE>
Core::Span<std::byte> AsBytes(Core::FixedArray<T, SIZE>& array) requires std::is_trivially_copyable_v<T> {
    return Core::AsWritableBytes(ToSpan(array));
}

template <typename T, u64 SIZE>
Core::Span<const std::byte> AsBytes(const Core::FixedArray<T, SIZE>& array) requires std::is_trivially_copyable_v<T> {
    return Core::AsBytes(ToSpan(array));
}

}    // namespace Core

namespace Core::IO {

template <typename T>
struct Serializer<Core::Array<T>> {
    static void serialize(OutputStream& stream, const Core::Array<T>& values) {
        stream.write(values.count());
        if constexpr(BinarySerializable<T>) {
            stream.write(Core::AsBytes(Core::ToSpan(values)));
        } else {
            for(const T& value : values) {
                Serializer<T>::serialize(stream, value);
            }
        }
    }
};

template <typename T, u64 SIZE>
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
        u64 elementCount = stream.read<u64>();
        Core::Array<T> value;

        if constexpr(BinarySerializable<T>) {
            stream.readInto<T>(value.insertUninitialized(elementCount));
        } else {
            for(size_t i = 0; i < elementCount; i++) {
                value.emplace(Deserializer<T>::deserialize(stream));
            }
        }
        return value;
    }
};

template <typename T, u64 SIZE>
struct Deserializer<Core::FixedArray<T, SIZE>> {
    static Core::FixedArray<T, SIZE> deserialize(InputStream& stream) {
        size_t elementCount = stream.read<Core::FixedArray<T, SIZE>::size_type>();
        ASSERT(elementCount == SIZE);

        Core::FixedArray<T, SIZE> value;

        if constexpr(std::is_trivial_v<T>) {
            stream.readInto(Core::AsBytes(value));
        } else {
            for(size_t i = 0; i < elementCount; i++) {
                value[i] = Deserializer<T>::deserialize(stream);
            }
        }

        return value;
    }
};

}    // namespace Core::IO

#include "core/containers/internal/Array.inl"
