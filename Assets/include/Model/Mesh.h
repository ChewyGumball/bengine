#pragma once

#include <vector>

#include "DllExport.h"

#include "VertexFormat.h"

namespace Assets {
struct ASSETS_API Mesh {
    VertexFormat vertexFormat;
    std::vector<float> vertexData;
    std::vector<uint32_t> indexData;
};
}    // namespace Assets