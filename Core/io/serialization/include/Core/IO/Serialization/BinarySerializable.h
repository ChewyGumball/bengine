#pragma once

#include <type_traits>

namespace Core::IO {

template <typename T>
concept BinarySerializable = std::is_trivially_constructible_v<T>&& std::is_trivially_copyable_v<T>;

}
