#pragma once

#include <concepts>
#include <span>
#include <type_traits>

namespace Core {


template <typename T>
class Span {
public:
    using ElementType = T;

    Span(T* elements, uint64_t count);
    Span(const Span<T>& other) = default;

    template <size_t EXTENT>
    Span(std::span<T, EXTENT> span);

    template <typename U, typename = std::enable_if_t<std::is_same_v<const U, T> && !std::is_same_v<U, T>>>
    Span(Span<U> other);

    Span<T>& operator=(const Span<T>& other) = default;

    template <typename U, typename = std::enable_if_t<std::is_same_v<const U, T> && !std::is_same_v<U, T>>>
    Span<T>& operator=(Span<U> other);

    [[nodiscard]] T& operator[](uint64_t i);
    [[nodiscard]] const T& operator[](uint64_t i) const;

    void truncateFront(uint64_t count);
    void truncateBack(uint64_t count);

    Span<T> first(uint64_t count);
    Span<T> last(uint64_t count);
    Span<T> subspan(uint64_t offset, uint64_t count) const;

    [[nodiscard]] uint64_t count() const;
    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] T* rawData();
    [[nodiscard]] const T* rawData() const;
    [[nodiscard]] T* begin();
    [[nodiscard]] const T* begin() const;
    [[nodiscard]] T* end();
    [[nodiscard]] const T* end() const;

private:
    uint64_t elementCount = 0;
    T* data               = nullptr;
};

template <typename T>
Span<const std::byte> AsBytes(Span<T> span) {
    return Span<const std::byte>(reinterpret_cast<const std::byte*>(span.rawData()), span.count() * sizeof(T));
}

template <typename T>
Span<std::byte> AsWritableBytes(Span<T> span) {
    return Span<std::byte>(reinterpret_cast<std::byte*>(span.rawData()), span.count() * sizeof(T));
}

template <typename T>
Core::Span<const T> ToSpan(std::initializer_list<T>& list) {
    return Core::Span<const T>(list.begin(), list.size());
}

template <typename T>
Core::Span<const std::byte> ToBytes(const T& value) requires std::is_trivially_copyable_v<T> {
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
    ->std::convertible_to<uint64_t>;
    { b.size() }
    ->std::convertible_to<uint64_t>;
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
