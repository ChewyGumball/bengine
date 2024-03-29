#pragma once

#include "core/containers/Span.h"

#include "core/assert/Assert.h"

namespace Core {

template <typename T>
Span<T>::Span(T* elements, u64 count) : elementCount(count), data(elements) {}

template <typename T>
template <typename U, typename>
Span<T>::Span(Span<U> other) : Span(other.rawData(), other.count()) {}

template <typename T>
template <size_t EXTENT>
Span<T>::Span(std::span<T, EXTENT> span) : Span(span.data(), span.size()) {}

template <typename T>
template <typename U, typename>
Span<T>& Span<T>::operator=(Span<U> other) {
    elementCount = other.elementCount;
    data         = other.data;
}

template <typename T>
T& Span<T>::operator[](u64 i) {
    ASSERT_WITH_MESSAGE(i < elementCount, "Index: {}, Count: {}", i, elementCount);
    return *(data + i);
}
template <typename T>
const T& Span<T>::operator[](u64 i) const {
    ASSERT_WITH_MESSAGE(i < elementCount, "Index: {}, Count: {}", i, elementCount);
    return *(data + i);
}

template <typename T>
void Span<T>::truncateFront(u64 count) {
    ASSERT_WITH_MESSAGE(count <= elementCount,
                        "Tried to truncate more than the span holds! Elements: {}, Truncate Amount: {}",
                        elementCount,
                        count);

    data += count;
    elementCount -= count;
}
template <typename T>
void Span<T>::truncateBack(u64 count) {
    ASSERT_WITH_MESSAGE(count <= elementCount,
                        "Tried to truncate more than the span holds! Elements: {}, Truncate Amount: {}",
                        elementCount,
                        count);
    elementCount -= count;
}

template <typename T>
Span<T> Span<T>::first(u64 count) {
    ASSERT_WITH_MESSAGE(count <= elementCount,
                        "Tried to get a subspan larger than the span! Elements: {}, Asked Count: {}",
                        elementCount,
                        count);
    return Span(data, count);
}
template <typename T>
Span<T> Span<T>::last(u64 count) {
    ASSERT_WITH_MESSAGE(count <= elementCount,
                        "Tried to get a subspan larger than the span! Elements: {}, Asked Count: {}",
                        elementCount,
                        count);
    return Span(data + elementCount - count, count);
}
template <typename T>
Span<T> Span<T>::subspan(u64 offset, u64 count) const {
    ASSERT_WITH_MESSAGE(offset + count <= elementCount,
                        "Tried to get a subspan larger than the span! Elements: {}, Asked Offset: {}, Asked Count: {}",
                        elementCount,
                        offset,
                        count);
    return Span(data + offset, count);
}

template <typename T>
u64 Span<T>::count() const {
    return elementCount;
}
template <typename T>
bool Span<T>::isEmpty() const {
    return elementCount == 0;
}
template <typename T>
T* Span<T>::rawData() {
    return data;
}
template <typename T>
const T* Span<T>::rawData() const {
    return data;
}
template <typename T>
T* Span<T>::begin() {
    return data;
}
template <typename T>
const T* Span<T>::begin() const {
    return data;
}
template <typename T>
T* Span<T>::end() {
    return data + elementCount;
}
template <typename T>
const T* Span<T>::end() const {
    return data + elementCount;
}

}    // namespace Core
