#pragma once

#include "Assets/Models/Mesh.h"

#include <filesystem>

namespace Assets::OBJ {
Mesh Import(const std::filesystem::path& filename);
}    // namespace Assets::OBJ