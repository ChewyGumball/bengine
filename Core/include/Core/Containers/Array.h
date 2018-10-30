#pragma once

#include <array>
#include <vector>

namespace Core {

template <typename T>
using Array = std::vector<T>;

template <typename T, uint64_t SIZE>
using FixedArray = std::array<T, SIZE>;

}    // namespace Core