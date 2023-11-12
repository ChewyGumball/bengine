#pragma once

#include <Core/Assert.h>

#include <memory>
#include <type_traits>

namespace Core::Algorithms {

// This function is only safe to use if destination is before source, or at least elementCount after source.
template <typename T>
void MoveElements(T* source, T* destination, uint64_t elementCount) {
    if(source == destination) {
        return;
    }

    if constexpr(std::is_trivially_copyable_v<T>) {
        std::memmove(destination, source, elementCount * sizeof(T));
        return;
    }

    ASSERT(destination < source || (destination - source) >= elementCount);

    T* sourceEnd = source + elementCount;

    if constexpr(std::is_move_assignable_v<T>) {
        std::move(source, sourceEnd, destination);
    } else if constexpr(std::is_copy_assignable_v<T>) {
        std::copy(source, sourceEnd, destination);
    } else {
        for(T *currentSource = source, *currentDestination = destination; currentSource < sourceEnd;
            currentSource++, currentDestination++) {
            std::destroy_at(currentDestination);
            if constexpr(std::is_move_constructible_v<T>) {
                std::construct_at(currentDestination, std::move(*currentSource));
            } else {
                std::construct_at(currentDestination, *currentSource);
            }
        }
    }
}

// This function is only safe to use if destination is after source, or at least elementCount before source.
template <typename T>
void MoveElementsBackwards(T* source, T* destination, uint64_t elementCount) {
    if(source == destination) {
        return;
    }

    if constexpr(std::is_trivially_copyable_v<T>) {
        std::memmove(destination, source, elementCount * sizeof(T));
        return;
    }

    ASSERT(destination > source || (source - destination) >= elementCount);

    T* sourceEnd      = source + elementCount;
    T* destinationEnd = destination + elementCount;

    if constexpr(std::is_move_assignable_v<T>) {
        std::move_backward(source, sourceEnd, destinationEnd);
    } else if constexpr(std::is_copy_assignable_v<T>) {
        std::copy_backward(source, sourceEnd, destinationEnd);
    } else {
        for(T *currentSource = sourceEnd - 1, *currentDestination = destinationEnd - 1; currentSource >= source;
            currentSource--, currentDestination--) {
            std::destroy_at(currentDestination);
            if constexpr(std::is_move_constructible_v<T>) {
                std::construct_at(currentDestination, std::move(*currentSource));
            } else {
                std::construct_at(currentDestination, *currentSource);
            }
        }
    }
}

template <typename T>
void DestroyElements(T* start, uint64_t elementCount) {
    if constexpr(!std::is_trivially_destructible_v<T>) {
        std::destroy(start, start + elementCount);
    }
}

}    // namespace Core::Algorithms
