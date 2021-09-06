#pragma once

#include <Core/IO/FileSystem/Path.h>

#include <Core/Containers/HashMap.h>

namespace Assets {
struct Model {
    Core::IO::Path meshPath;
    Core::HashMap<std::string, Core::IO::Path> materials;
};
}    // namespace Assets