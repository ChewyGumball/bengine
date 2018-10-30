#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <set>
#include <unordered_set>

namespace Core::Algorithms {
template <typename T, typename U = typename T::value_type>
bool Contains(const T& container, const U& value) {
    return std::find(std::begin(container), std::end(container), value) != std::end(container);
}

template <typename T, typename U>
bool ContainsAll(const T& container, const U& elements) {
    for(const auto& element : elements) {
        if(!Contains(container, element)) {
            return false;
        }
    }

    return true;
}

template <typename T, typename U = typename T::value_type>
std::optional<std::reference_wrapper<const U>> Find(T& container, const U& value) {
    auto found = std::find(std::begin(container), std::end(container), value);
    if(found != std::end(container)) {
        return *found;
    }

    return std::nullopt;
}

template <typename T, typename P, typename U = typename T::value_type>
std::optional<std::reference_wrapper<const U>> FindIf(const T& container, P predicate) {
    auto found = std::find_if(std::begin(container), std::end(container), predicate);
    if(found != std::end(container)) {
        return *found;
    }

    return std::nullopt;
}

template <typename T, typename U = typename T::value_type>
std::optional<std::reference_wrapper<const U>> Prefer(T& container, const U& value) {
    return Find(container, value);
}

template <typename T, typename U = typename T::value_type, typename... REST>
std::optional<std::reference_wrapper<const U>> Prefer(T& container, const U& firstValue, const REST&... rest) {
    auto found = Find(container, firstValue);
    if(found.has_value()) {
        return found;
    }

    return Prefer(container, rest...);
}

template <typename T, typename U = typename T::value_type, typename N, size_t S, typename MAPPING_FUNCTION>
void Map(const T& before, std::array<N, S> after, MAPPING_FUNCTION mapper) {
    std::transform(std::begin(before), std::end(before), std::begin(after), mapper);
}

template <typename T,
          typename U = typename T::value_type,
          typename M,
          typename N = typename M::value_type,
          typename MAPPING_FUNCTION>
void Map(const T& before, M& after, MAPPING_FUNCTION mapper) {
    std::transform(std::begin(before), std::end(before), std::inserter(after, std::end(after)), mapper);
}

template <typename T, typename U = typename T::value_type, typename P>
T Filter(const T& container, P predicate) {
    T filteredContainer;
    std::copy_if(std::begin(container),
                 std::end(container),
                 std::inserter(filteredContainer, std::end(filteredContainer)),
                 predicate);
    return filteredContainer;
}

template <typename T>
bool AllEqual(const std::vector<T>& container) {
    if(container.empty()) {
        return true;
    }

    return std::all_of(container.begin(), container.end(), [&](const T& element) { return element == container[0]; });
}
}    // namespace Core::Algorithms