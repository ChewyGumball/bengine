#pragma once

#include <unordered_set>

namespace Core {
template <typename T>
using HashSet = std::unordered_set<T>;
}