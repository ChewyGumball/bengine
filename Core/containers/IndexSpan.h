#pragma once

#include "core/assert/Assert.h"
#include "core/containers/Array.h"
#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"


namespace Core {

struct IndexSpan {
    uint64_t start;
    uint64_t count;

    IndexSpan(uint64_t count) : IndexSpan(0, count) {}
    IndexSpan(uint64_t start, uint64_t count) : start(start), count(count) {}

    uint64_t operator[](uint64_t index) const {
        ASSERT(index < count);
        return start + index;
    }

    IndexSpan slice(uint64_t offset, uint64_t count) {
        ASSERT(count + offset < this->count);
        return IndexSpan(start + offset, count);
    }
};
}    // namespace Core

namespace Core::IO {
template <>
struct Serializer<Core::IndexSpan> {
    static void serialize(OutputStream& stream, const Core::IndexSpan& value) {
        stream.write(value.start);
        stream.write(value.count);
    }
};

template <>
struct Deserializer<Core::IndexSpan> {
    static Core::IndexSpan deserialize(InputStream& stream) {
        uint64_t start = stream.read<uint64_t>();
        uint64_t count = stream.read<uint64_t>();
        return Core::IndexSpan(start, count);
    }
};
}    // namespace Core::IO
