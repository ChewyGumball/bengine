#pragma once

#include <filesystem>
#include <vector>

#include "DllExport.h"

#include "Model/Mesh.h"

namespace Assets::OBJ {
struct ASSETS_API OBJModel {
    std::vector<Mesh> meshes;
};

ASSETS_API OBJModel Import(const std::filesystem::path& filename);
}    // namespace Assets::OBJ