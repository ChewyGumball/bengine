#pragma once

#include <Core/Containers/Array.h>

#include <Core/Containers/Span.h>

namespace Core {
template <typename T>
Array<T>::Array(uint64_t initialCapacity) : capacity(initialCapacity) {
    data = reinterpret_cast<T*>(malloc(capacity * ElementSize));
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
        std::memcpy(data, other.data, elementCount * ElementSize);
    } else {
        for(uint64_t i = 0; i < elementCount; i++) {
            new(data + i) T(other[i]);
        }
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
        destructElements(0, elementCount);
        ensureCapacity(other.elementCount);

        if constexpr(std::is_trivially_copyable_v<T>) {
            std::memcpy(data, other.data, other.elementCount);
        } else {
            for(uint64_t i = 0; i < other.elementCount; i++) {
                new(data + i) T(other[i]);
            }
        }

        elementCount = other.elementCount;
    }

    return *this;
}

template <typename T>
Array<T>& Array<T>::operator=(Array<T>&& other) {
    if(&other != this) {
        destructElements(0, elementCount);
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
    destructElements(0, elementCount);
    free(data);
}

template <typename T>
T& Array<T>::operator[](uint64_t i) {
    ASSERT(i < elementCount);
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

    ensureCapacity(elementCount + 1);

    if(index < elementCount) {
        moveElements(index, index + 1, elementCount - index);
    }

    T* newElement = new(data + index) T(std::forward<ARGS>(args)...);
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
        std::memcpy(data + elementCount, elements.data(), elements.size() * ElementSize);
    } else {
        for(uint64_t i = 0; i < elements.size(); i++) {
            new(data + elementCount + i) T(elements[i]);
        }
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
    moveElements(index + 1, index, elementCount - index - 1);
    elementCount--;
}

template <typename T>
void Array<T>::clear() {
    destructElements(0, elementCount);
    elementCount = 0;
}

template <typename T>
uint64_t Array<T>::count() const {
    return elementCount;
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
        std::memcpy(newData, data, elementCount * ElementSize);
    } else {
        for(uint64_t i = 0; i < elementCount; i++) {
            if constexpr(std::is_move_constructible_v<T>) {
                new(newData + i) T(std::move(*(data + i)));
            } else {
                new(newData + i) T(*(data + i));
            }
        }
    }

    destructElements(0, elementCount);

    free(data);
    data = newData;
}

template <typename T>
void Array<T>::destructElements(uint64_t startIndex, uint64_t elementsToDestruct) {
    if constexpr(std::is_trivially_destructible_v<T>) {
        return;
    } else {
        uint64_t currentIndex = startIndex;
        while(currentIndex < startIndex + elementsToDestruct) {
            (data + currentIndex++)->~T();
        }
    }
}

template <typename T>
void Array<T>::moveElements(uint64_t sourceIndex, uint64_t destinationIndex, uint64_t elementCount) {
    if(sourceIndex == destinationIndex) {
        return;
    }

    if constexpr(std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>) {
        std::memmove(data + destinationIndex, data + sourceIndex, elementCount * ElementSize);
    } else {
        for(uint64_t i = 0; i < elementCount; i++) {
            uint64_t offset;
            if(sourceIndex < destinationIndex) {
                offset = elementCount - i - 1;
            } else {
                offset = i;
            }

            T* sourceElement      = data + sourceIndex + offset;
            T* destinationElement = data + destinationIndex + offset;

            if constexpr(!std::is_trivially_destructible_v<T>) {
                destinationElement->~T();
            }

            if constexpr(std::is_trivially_copyable_v<T>) {
                std::memcpy(destinationElement, sourceElement, ElementSize);
            } else {
                if constexpr(std::is_move_constructible_v<T>) {
                    new(destinationElement) T(std::move(*sourceElement));
                } else {
                    new(destinationElement) T(*sourceElement);
                }
            }

            if constexpr(!std::is_trivially_destructible_v<T>) {
                (data + sourceIndex + i)->~T();
            }
        }
    }
}
}    // namespace Core
