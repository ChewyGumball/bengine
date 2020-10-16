#pragma once

#include <concepts>
#include <span>

namespace Core {

template <typename T, typename U>
concept Spannable = requires(T a, const T b) {
    { a.data() }
    ->std::same_as<U*>;
    { b.data() }
    ->std::same_as<const U*>;

    { a.size() }
    ->std::convertible_to<std::size_t>;
    { b.size() }
    ->std::convertible_to<std::size_t>;
};

template <typename T>
std::span<const std::byte, sizeof(T)> ToBytes(const T& value) requires std::is_trivially_copyable_v<T> {
    return std::as_bytes(std::span<const T, 1>(&value, 1));
}

template <Spannable T>
auto ToSpan(T& a) {
    return std::span(a.data(), a.size());
}

template <Spannable T>
auto ToSpan(const T& a) {
    return std::span(a.data(), a.size());
}

template <typename T>
std::span<const T> ToSpan(std::initializer_list<T>& list) {
    return std::span<const T>(list.begin(), list.end());
}

}    // namespace Core
