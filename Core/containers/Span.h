#pragma once

#include <concepts>
#include <span>
#include <type_traits>

#include "core/Types.h"

namespace Core {


template <typename T>
class Span {
public:
    using ElementType = T;

    Span(T* elements, u64 count);
    Span(const Span<T>& other) = default;

    template <size_t EXTENT>
    Span(std::span<T, EXTENT> span);

    template <typename U, typename = std::enable_if_t<std::is_same_v<const U, T> && !std::is_same_v<U, T>>>
    Span(Span<U> other);

    Span<T>& operator=(const Span<T>& other) = default;

    template <typename U, typename = std::enable_if_t<std::is_same_v<const U, T> && !std::is_same_v<U, T>>>
    Span<T>& operator=(Span<U> other);

    [[nodiscard]] T& operator[](u64 i);
    [[nodiscard]] const T& operator[](u64 i) const;

    void truncateFront(u64 count);
    void truncateBack(u64 count);

    Span<T> first(u64 count);
    Span<T> last(u64 count);
    Span<T> subspan(u64 offset, u64 count) const;

    [[nodiscard]] u64 count() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] T* rawData();
    [[nodiscard]] const T* rawData() const;
    [[nodiscard]] T* begin();
    [[nodiscard]] const T* begin() const;
    [[nodiscard]] T* end();
    [[nodiscard]] const T* end() const;

private:
    u64 elementCount = 0;
    T* data          = nullptr;
};

template <typename T>
Span<const byte> AsBytes(Span<T> span) {
    return Span<const byte>(reinterpret_cast<const byte*>(span.rawData()), span.count() * sizeof(T));
}

template <typename T>
Span<byte> AsWritableBytes(Span<T> span) {
    return Span<byte>(reinterpret_cast<byte*>(span.rawData()), span.count() * sizeof(T));
}

template <typename T>
Core::Span<const T> ToSpan(std::initializer_list<T>& list) {
    return Core::Span<const T>(list.begin(), list.size());
}

template <typename T>
Core::Span<const byte> ToBytes(const T& value) requires std::is_trivially_copyable_v<T> {
    return AsBytes(Span<const T>(&value, 1));
}

template <typename T>
concept IsPointer = std::is_pointer_v<T>;

template <typename T>
concept Spannable = requires(T a, const T b) {
    { a.data() }
    ->IsPointer;
    { b.data() }
    ->IsPointer;

    { a.size() }
    ->std::convertible_to<u64>;
    { b.size() }
    ->std::convertible_to<u64>;
};

template <Spannable T>
auto ToSpan(T& a) {
    return Span(a.data(), a.size());
}

template <Spannable T>
auto ToSpan(const T& a) {
    return Span(a.data(), a.size());
}

template <Spannable T>
auto AsBytes(T& a) {
    return AsWritableBytes(ToSpan(a));
}

template <Spannable T>
auto AsBytes(const T& a) {
    return AsBytes(ToSpan(a));
}

}    // namespace Core

#include "core/containers/internal/Span.inl"
