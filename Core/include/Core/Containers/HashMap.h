#pragma once

#include <unordered_map>

namespace Core {
template <typename KEY, typename VALUE>
using HashMap = std::unordered_map<KEY, VALUE>;
}