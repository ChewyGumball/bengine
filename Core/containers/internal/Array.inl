#pragma once

#include "core/containers/Array.h"

#include "core/algorithms/Memory.h"
#include "core/containers/Span.h"

namespace Core {

template <typename T>
Array<T>::Array(u64 initialCapacity) : capacity(initialCapacity) {
    data = reinterpret_cast<T*>(malloc(capacity * ElementSize));
}

template <typename T>
Array<T>::Array(Core::Span<T> elementsToCopy) : capacity(elementsToCopy.count()) {
    data = reinterpret_cast<T*>(malloc(capacity * ElementSize));
    insertAll(elementsToCopy);
}

template <typename T>
Array<T>::Array(const T& original, u64 repeatCount) : Array() {
    ensureCapacity(repeatCount);
    for(u64 i = 0; i < repeatCount; i++) {
        emplace(original);
    }
}

template <typename T>
Array<T>::Array(std::initializer_list<T> initializerList) : Array() {
    insertAll(Core::ToSpan(initializerList));
}

template <typename T>
Array<T>::Array(const Array<T>& other)
  : capacity(other.capacity),
    elementCount(other.elementCount),
    data(reinterpret_cast<T*>(malloc(capacity * ElementSize))) {
    if constexpr(std::is_trivially_copyable_v<T>) {
        CopyElementMemory(data, other.data, elementCount);
    } else {
        std::uninitialized_copy_n(other.data, elementCount, data);
    }
}

template <typename T>
Array<T>::Array(Array<T>&& other) : capacity(other.capacity), elementCount(other.elementCount), data(other.data) {
    other.capacity     = 0;
    other.elementCount = 0;
    other.data         = nullptr;
}

template <typename T>
Array<T>& Array<T>::operator=(const Array<T>& other) {
    if(&other != this) {
        destructAllElements();
        ensureCapacity(other.elementCount);

        if constexpr(std::is_trivially_copyable_v<T>) {
            CopyElementMemory(data, other.data, other.elementCount);
        } else {
            std::uninitialized_copy_n(other.data, other.elementCount, data);
        }

        elementCount = other.elementCount;
    }

    return *this;
}

template <typename T>
Array<T>& Array<T>::operator=(Array<T>&& other) {
    if(&other != this) {
        destructAllElements();
        free(data);

        capacity     = other.capacity;
        elementCount = other.elementCount;
        data         = other.data;

        other.capacity     = 0;
        other.elementCount = 0;
        other.data         = nullptr;
    }

    return *this;
}

template <typename T>
Array<T>::~Array() {
    destructAllElements();
    free(data);
}

template <typename T>
T& Array<T>::operator[](u64 i) {
    ASSERT_WITH_MESSAGE(i < elementCount, "Index: {}, Count: {}", i, elementCount);
    return *(data + i);
}

template <typename T>
const T& Array<T>::operator[](u64 i) const {
    ASSERT_WITH_MESSAGE(i < elementCount, "Index: {}, Count: {}", i, elementCount);
    return *(data + i);
}

template <typename T>
template <typename... ARGS>
T& Array<T>::emplaceAt(u64 index, ARGS&&... args) {
    ASSERT(index <= elementCount);

    if(index < elementCount) {
        shiftElementsRight(index, 1);
    } else {
        ensureCapacity(elementCount + 1);
    }

    T* newElement = std::construct_at(data + index, std::forward<ARGS>(args)...);
    elementCount++;
    return *newElement;
}

template <typename T>
template <typename... ARGS>
T& Array<T>::emplace(ARGS&&... args) {
    return emplaceAt(elementCount, std::forward<ARGS>(args)...);
}

template <typename T>
T& Array<T>::insertAt(u64 index, const T& elementToInsert) {
    return emplaceAt(index, elementToInsert);
}

template <typename T>
T& Array<T>::insertAt(u64 index, T&& elementToInsert) {
    if constexpr(std::is_move_constructible_v<T>) {
        return emplaceAt(index, std::move(elementToInsert));
    } else {
        return emplaceAt(index, elementToInsert);
    }
}

template <typename T>
T& Array<T>::insert(const T& elementToInsert) {
    return emplace(elementToInsert);
}

template <typename T>
T& Array<T>::insert(T&& elementToInsert) {
    if constexpr(std::is_move_constructible_v<T>) {
        return emplace(std::forward<T>(elementToInsert));
    } else {
        return emplace(elementToInsert);
    }
}

template <typename T>
template <typename U>
Core::Span<T> Array<T>::insertAll(Core::Span<U> elements) {
    ensureCapacity(elementCount + elements.count());
    if constexpr(std::is_same_v<std::remove_cv_t<U>, T> && std::is_trivially_copyable_v<T>) {
        CopyElementMemory(data + elementCount, elements.rawData(), elements.count());
    } else {
        std::uninitialized_copy_n(elements.rawData(), elements.count(), end());
    }

    elementCount += elements.count();

    return Core::Span<T>(data + elementCount - elements.count(), elements.count());
}

template <typename T>
Core::Span<T> Array<T>::insertUninitialized(u64 newElementCount) {
    ensureCapacity(elementCount + newElementCount);
    elementCount += newElementCount;

    return Core::Span<T>(data + elementCount - newElementCount, newElementCount);
}

template <typename T>
void Array<T>::eraseAt(u64 index, u64 elementsToErase) {
    shiftElementsLeft(index + elementsToErase, elementsToErase);
}

template <typename T>
void Array<T>::clear() {
    destructAllElements();
    elementCount = 0;
}

template <typename T>
u64 Array<T>::count() const {
    return elementCount;
}

template <typename T>
u64 Array<T>::totalCapacity() const {
    return capacity;
}

template <typename T>
u64 Array<T>::unusedCapacity() const {
    return capacity - elementCount;
}

template <typename T>
bool Array<T>::isEmpty() const {
    return elementCount == 0;
}

template <typename T>
T* Array<T>::rawData() {
    return data;
}

template <typename T>
const T* Array<T>::rawData() const {
    return data;
}

template <typename T>
T* Array<T>::begin() {
    return data;
}

template <typename T>
const T* Array<T>::begin() const {
    return data;
}

template <typename T>
T* Array<T>::end() {
    return data + elementCount;
}

template <typename T>
const T* Array<T>::end() const {
    return data + elementCount;
}

template <typename T>
void Array<T>::ensureCapacity(u64 requiredCapacity) {
    if(requiredCapacity <= capacity) {
        return;
    }

    while(capacity < requiredCapacity) {
        capacity = std::max(static_cast<u64>(capacity * GrowthFactor), MinimumCapacity);
    }

    T* newData = reinterpret_cast<T*>(malloc(capacity * ElementSize));
    if constexpr(std::is_trivially_copyable_v<T>) {
        CopyElementMemory(newData, data, elementCount);
    } else if constexpr(std::is_move_constructible_v<T>) {
        std::uninitialized_move_n(data, elementCount, newData);
    } else {
        std::uninitialized_copy_n(data, elementCount, newData);
    }

    destructAllElements();

    free(data);
    data = newData;
}

template <typename T>
void Array<T>::destructAllElements() {
    Core::Algorithms::DestroyElements(data, elementCount);
}

template <typename T>
void Array<T>::CopyElementMemory(T* destination, const T* source, u64 elementCount) {
    std::memcpy(destination, source, elementCount * ElementSize);
}

template <typename T>
void Array<T>::MoveElementMemory(T* destination, const T* source, u64 elementCount) {
    std::memmove(destination, source, elementCount * ElementSize);
}


template <typename T>
void Array<T>::shiftElementsLeft(u64 startIndex, u64 distance) {
    ASSERT(distance <= startIndex);

    T* source      = data + startIndex;
    T* destination = source - distance;

    Core::Algorithms::MoveElements(source, destination, elementCount - startIndex);

    elementCount -= distance;
    Core::Algorithms::DestroyElements(data + elementCount, distance);
}

template <typename T>
void Array<T>::shiftElementsRight(u64 startIndex, u64 distance) {
    ensureCapacity(elementCount + distance);

    T* source      = data + startIndex;
    T* destination = source + distance;

    Core::Algorithms::MoveElementsBackwards(source, destination, elementCount - startIndex);
    Core::Algorithms::DestroyElements(source, distance);
}

}    // namespace Core
