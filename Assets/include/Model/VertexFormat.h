#pragma once

#include <optional>

#include "DllExport.h"

namespace Assets {
struct ASSETS_API VertexFormat {
    uint8_t elementCount = 1;

    uint8_t positionOffset;
    std::optional<uint8_t> colourOffset;
    std::optional<uint8_t> normalOffset;
    std::optional<uint8_t> textureCoordinateOffset;
    std::optional<uint8_t> textureCoordinate2Offset;
};
}    // namespace Assets