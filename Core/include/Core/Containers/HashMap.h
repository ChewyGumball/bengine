#pragma once

#include <unordered_map>

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
        for (auto& it : value) {
            stream.write(it.first);
            stream.write(it.second);
        }
    }
};
}