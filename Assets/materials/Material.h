#pragma once

#include "core/containers/Array.h"
#include "core/containers/HashMap.h"

#include "core/io/file_system/Path.h"

namespace Assets {
struct TextureInput {
    Core::IO::Path path;
};

struct BufferInput {
    Core::Array<std::byte> data;
};

using MaterialInput = std::variant<TextureInput, BufferInput>;

struct Material {
    Core::HashMap<std::string, MaterialInput> inputs;
    Core::IO::Path shaderPath;
};
}    // namespace Assets