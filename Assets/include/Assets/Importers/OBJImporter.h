#pragma once

#include "Assets/Model/Mesh.h"

#include <Core/Containers/Array.h>

#include <filesystem>

namespace Assets::OBJ {
Mesh Import(const std::filesystem::path& filename);
}    // namespace Assets::OBJ