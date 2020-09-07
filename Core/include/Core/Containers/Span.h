#pragma once

#include <span>

namespace Core {

template <typename T>
std::span<const std::byte, sizeof(T)> ToBytes(const T& value) {
    static_assert(std::is_trivially_copyable_v<T>);
    return std::as_bytes(std::span<const T, 1>(&value, 1));
}

template <typename T>
std::span<T> ToSpan(std::basic_string<T>& string) {
    return std::span<T>(string.data(), string.size());
}

template <typename T>
std::span<const T> ToSpan(const std::basic_string<T>& string) {
    return std::span<T>(string.data(), string.size());
}

template <typename T>
std::span<const T> ToSpan(std::initializer_list<T>& list) {
    return std::span<const T>(list.begin(), list.end());
}

}    // namespace Core
