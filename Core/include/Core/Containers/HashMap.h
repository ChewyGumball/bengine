#pragma once

#include <unordered_map>

#include "Core/IO/InputStream.h"
#include "Core/IO/OutputStream.h"

namespace Core {
template <typename KEY, typename VALUE>
using HashMap = std::unordered_map<KEY, VALUE>;
}

namespace Core::IO {
template <typename KEY, typename VALUE>
struct Serializer<Core::HashMap<KEY, VALUE>> {
    static void serialize(Core::IO::OutputStream& stream, const Core::HashMap<KEY, VALUE>& value) {
        stream.write(value.size());
        for(auto& it : value) {
            stream.write(it.first);
            stream.write(it.second);
        }
    }
};

template <typename KEY, typename VALUE>
struct Deserializer<Core::HashMap<KEY, VALUE>> {
    static Core::HashMap<KEY, VALUE> deserialize(Core::IO::InputStream& stream) {
        Core::HashMap<KEY, VALUE> value;

        size_t entryCount = stream.read<size_t>();
        for(size_t i = 0; i < entryCount; i++) {
            KEY k   = stream.read<KEY>();
            VALUE v = stream.read<VALUE>();
            value.emplace(std::move(k), std::move(v));
        }

        return value;
    }
};
}    // namespace Core::IO
