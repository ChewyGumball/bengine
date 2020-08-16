#pragma once

#include <optional>
#include <type_traits>

namespace Core::Algorithms {
template <typename T, typename MAPPING_FUNCTION>
auto Map(const std::optional<T>& optional, const MAPPING_FUNCTION& mapper)
      -> std::optional<std::invoke_result_t<MAPPING_FUNCTION, const T&>> {
    if(!optional) {
        return std::nullopt;
    } else {
        return mapper(*optional);
    }
}
}    // namespace Core::Algorithms