#pragma once

#include <map>
namespace Core {
template <typename KEY, typename VALUE>
using OrderedMap = std::map<KEY, VALUE>;
}