#pragma once

#include <Core/Containers/Array.h>
#include <Core/Containers/HashMap.h>

#include <Core/IO/FileSystem/Path.h>

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