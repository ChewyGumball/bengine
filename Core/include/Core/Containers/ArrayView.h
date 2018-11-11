#pragma once

#include <assert.h>

#include "Array.h"

namespace Core {

template <typename T>
struct ArrayView {
    T* data;
    uint64_t count;

    ArrayView(Core::Array<T>& a) : data(a.data()), count(a.size()) {}

    template <uint64_t SIZE>
    ArrayView(Core::FixedArray<T, SIZE>& a) : data(a.data()), count(a.size()) {}

    T& operator[](uint64_t index) {
        assert(index < count);
        return *(data + index);
    }
    const T& operator[](uint64_t index) const {
        assert(index < count);
        return *(data + index);
    }

    ArrayView slice(uint64_t offset, uint64_t count) {
        assert(count + offset <= this->count);
        return ArrayView{data + offset, count};
    }
};

struct IndexArrayView {
    uint64_t start;
    uint64_t count;

    IndexArrayView(uint64_t count) : IndexArrayView(0, count) {}
    IndexArrayView(uint64_t start, uint64_t count) : start(start), count(count) {}

    uint64_t operator[](uint64_t index) const {
        assert(index < count);
        return start + index;
    }

    IndexArrayView slice(uint64_t offset, uint64_t count) {
        assert(count + offset < this->count);
        return IndexArrayView(start + offset, count);
    }
};
}    // namespace Core

namespace Core::IO {

template <>
struct Serializer<Core::IndexArrayView> {
    static void serialize(OutputStream& stream, const Core::IndexArrayView& value) {
        stream.write(value.start);
        stream.write(value.count);
    }
};
}    // namespace Core::IO