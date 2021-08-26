#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <set>
#include <unordered_set>

#include <Core/Containers/Array.h>
#include <Core/Status/StatusOr.h>

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


template <typename T,
          typename MAPPING_FUNCTION,
          typename U = std::invoke_result_t<MAPPING_FUNCTION, std::add_lvalue_reference_t<std::add_const_t<T>>>>
Core::Array<U> Map(const Core::Array<T>& collection, MAPPING_FUNCTION transform) {
    static_assert(std::is_invocable_r_v<U, MAPPING_FUNCTION, std::add_lvalue_reference_t<std::add_const_t<T>>>);

    Core::Array<U> after;
    for(const T& element : collection) {
        after.emplace(transform(element));
    }
    return after;
}

template <
      typename T,
      typename MAPPING_FUNCTION,
      typename STATUS_TYPE = std::invoke_result_t<MAPPING_FUNCTION, std::add_lvalue_reference_t<std::add_const_t<T>>>,
      typename U           = STATUS_TYPE::value_type>
Core::StatusOr<Core::Array<U>> MapWithStatus(const Core::Array<T>& collection, MAPPING_FUNCTION transform) {
    static_assert(std::is_invocable_r_v<U, MAPPING_FUNCTION, std::add_lvalue_reference_t<std::add_const_t<T>>>);

    Core::Array<U> after;
    for(const T& element : collection) {
        ASSIGN_OR_RETURN(U transformedElement, transform(element));
        after.emplace(std::move(element));
    }
    return after;
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