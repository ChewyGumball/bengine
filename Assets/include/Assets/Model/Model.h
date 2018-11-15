#pragma once

#include <Core/Containers/HashMap.h>

#include "Mesh.h"
#include "Assets/Material/Material.h"

namespace Assets {
struct Model {
    Mesh mesh;
    Core::HashMap<std::string, Material> materials;
};
}