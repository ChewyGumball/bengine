#pragma once

#include "core/io/file_system/Path.h"

#include "core/containers/HashMap.h"

namespace Assets {
struct Model {
    Core::IO::Path meshPath;
    Core::HashMap<std::string, Core::IO::Path> materials;
};
}    // namespace Assets