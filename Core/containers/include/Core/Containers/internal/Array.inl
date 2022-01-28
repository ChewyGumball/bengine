#pragma once

#include <Core/Containers/Array.h>

#include <Core/Algorithms/Memory.h>
#include <Core/Containers/Span.h>

namespace Core {

template <typename T>
Array<T>::Array(uint64_t initialCapacity) : capacity(initialCapacity) {
    data = reinterpret_cast<T*>(malloc(capacity * ElementSize));
}

template <typename T>
template <std::size_t EXTENT>
Array<T>::Array(std::span<T, EXTENT> elementsToCopy) : capacity(elementsToCopy.size()) {
    data = reinterpret_cast<T*>(malloc(capacity * ElementSize));
    insertAll(elementsToCopy);
}

template <typename T>
Array<T>::Array(const T& original, uint64_t repeatCount) : Array() {
    ensureCapacity(repeatCount);
    for(uint64_t i = 0; i < repeatCount; i++) {
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
T& Array<T>::operator[](uint64_t i) {
    ASSERT_WITH_MESSAGE(i < elementCount, "Index: {}, Count: {}", i, elementCount);
    return *(data + i);
}

template <typename T>
const T& Array<T>::operator[](uint64_t i) const {
    ASSERT(i < elementCount);
    return *(data + i);
}

template <typename T>
template <typename... ARGS>
T& Array<T>::emplaceAt(uint64_t index, ARGS&&... args) {
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
T& Array<T>::insertAt(uint64_t index, const T& elementToInsert) {
    return emplaceAt(index, elementToInsert);
}

template <typename T>
T& Array<T>::insertAt(uint64_t index, T&& elementToInsert) {
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
template <typename U, std::size_t EXTENT>
std::span<T, EXTENT> Array<T>::insertAll(std::span<U, EXTENT> elements) {
    ensureCapacity(elementCount + elements.size());
    if constexpr(std::is_same_v<std::remove_cv_t<U>, T> && std::is_trivially_copyable_v<T>) {
        CopyElementMemory(data + elementCount, elements.data(), elements.size());
    } else {
        std::uninitialized_copy_n(elements.data(), elements.size(), end());
    }

    elementCount += elements.size();

    return std::span<T, EXTENT>(data + elementCount - elements.size(), elements.size());
}

template <typename T>
std::span<T> Array<T>::insertUninitialized(uint64_t newElementCount) {
    ensureCapacity(elementCount + newElementCount);
    elementCount += newElementCount;

    return std::span<T>(data + elementCount - newElementCount, newElementCount);
}

template <typename T>
void Array<T>::eraseAt(uint64_t index) {
    shiftElementsLeft(index + 1, 1);
}

template <typename T>
void Array<T>::clear() {
    destructAllElements();
    elementCount = 0;
}

template <typename T>
uint64_t Array<T>::count() const {
    return elementCount;
}

template <typename T>
uint64_t Array<T>::unusedCapacity() const {
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
void Array<T>::ensureCapacity(uint64_t requiredCapacity) {
    if(requiredCapacity <= capacity) {
        return;
    }

    while(capacity < requiredCapacity) {
        capacity = std::max(static_cast<uint64_t>(capacity * GrowthFactor), MinimumCapacity);
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
void Array<T>::CopyElementMemory(T* destination, const T* source, uint64_t elementCount) {
    std::memcpy(destination, source, elementCount * ElementSize);
}

template <typename T>
void Array<T>::MoveElementMemory(T* destination, const T* source, uint64_t elementCount) {
    std::memmove(destination, source, elementCount * ElementSize);
}


template <typename T>
void Array<T>::shiftElementsLeft(uint64_t startIndex, uint64_t distance) {
    ASSERT(distance <= startIndex);

    T* source      = data + startIndex;
    T* destination = source - distance;

    Core::Algorithms::MoveElements(source, destination, elementCount - startIndex);

    elementCount -= distance;
    Core::Algorithms::DestroyElements(data + elementCount, distance);
}

template <typename T>
void Array<T>::shiftElementsRight(uint64_t startIndex, uint64_t distance) {
    ensureCapacity(elementCount + distance);

    T* source      = data + startIndex;
    T* destination = source + distance;

    Core::Algorithms::MoveElementsBackwards(source, destination, elementCount - startIndex);
    Core::Algorithms::DestroyElements(source, distance);
}

}    // namespace Core
