#pragma once

#include "core/Types.h"
#include "core/assert/Assert.h"
#include "core/containers/Array.h"
#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"


namespace Core {

struct IndexSpan {
    u64 start;
    u64 count;

    IndexSpan(u64 count) : IndexSpan(0, count) {}
    IndexSpan(u64 start, u64 count) : start(start), count(count) {}

    u64 operator[](u64 index) const {
        ASSERT(index < count);
        return start + index;
    }

    IndexSpan slice(u64 offset, u64 count) {
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
        u64 start = stream.read<u64>();
        u64 count = stream.read<u64>();
        return Core::IndexSpan(start, count);
    }
};
}    // namespace Core::IO
