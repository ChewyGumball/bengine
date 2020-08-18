#pragma once


#include <Core/Containers/Array.h>

#include <Core/Assert.h>
#include <Core/IO/InputStream.h>
#include <Core/IO/OutputStream.h>


namespace Core {

template <typename T>
struct ArrayView {
    T* data;
    uint64_t count;

    ArrayView(T* data, uint64_t count) : data(data), count(count) {}

    ArrayView(Core::Array<T>& a) : data(a.data()), count(a.size()) {}

    template <uint64_t SIZE>
    ArrayView(Core::FixedArray<T, SIZE>& a) : data(a.data()), count(a.size()) {}

    T& operator[](uint64_t index) {
        ASSERT(index < count);
        return *(data + index);
    }
    const T& operator[](uint64_t index) const {
        ASSERT(index < count);
        return *(data + index);
    }

    ArrayView slice(uint64_t offset, uint64_t count) {
        ASSERT(count + offset <= this->count);
        return ArrayView{data + offset, count};
    }
};

struct IndexArrayView {
    uint64_t start;
    uint64_t count;

    IndexArrayView(uint64_t count) : IndexArrayView(0, count) {}
    IndexArrayView(uint64_t start, uint64_t count) : start(start), count(count) {}

    uint64_t operator[](uint64_t index) const {
        ASSERT(index < count);
        return start + index;
    }

    IndexArrayView slice(uint64_t offset, uint64_t count) {
        ASSERT(count + offset < this->count);
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

template <>
struct Deserializer<Core::IndexArrayView> {
    static Core::IndexArrayView deserialize(InputStream& stream) {
        return Core::IndexArrayView(stream.read<uint64_t>(), stream.read<uint64_t>());
    }
};
}    // namespace Core::IO
