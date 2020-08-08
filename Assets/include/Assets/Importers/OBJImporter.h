#pragma once

#include <filesystem>

#include <Core/Containers/Array.h>


#include "Assets/Model/Mesh.h"

namespace Assets::OBJ {
Mesh Import(const std::filesystem::path& filename);
}    // namespace Assets::OBJ