#pragma once

#include <filesystem>

#include <Core/Containers/Array.h>

#include "DllExport.h"

#include "Assets/Model/Mesh.h"

namespace Assets::OBJ {
ASSETS_API Mesh Import(const std::filesystem::path& filename);
}    // namespace Assets::OBJ