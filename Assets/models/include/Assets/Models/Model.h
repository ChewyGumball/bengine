#pragma once

#include "Assets/Material/Material.h"
#include "Assets/Models/Mesh.h"

#include <Core/Containers/HashMap.h>

namespace Assets {
struct Model {
    Mesh mesh;
    Core::HashMap<std::string, Material> materials;
};
}    // namespace Assets