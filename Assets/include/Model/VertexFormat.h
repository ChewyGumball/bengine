#pragma once

#include "DllExport.h"

#include <Core/Containers/HashMap.h>

namespace Assets {

using VertexUsageName = uint32_t;

namespace VertexUsage {
    constexpr VertexUsageName POSITION = 0;
    constexpr VertexUsageName NORMAL   = 1;
    constexpr VertexUsageName COLOUR   = 2;
    constexpr VertexUsageName TEXTURE  = 3;
    constexpr VertexUsageName TEXTURE2 = 4;
}    // namespace VertexUsage

enum VertexPropertyFormat { FLOAT_32, FLOAT_64, INT_16, INT_32, UINT_16, UINT_32 };

struct ASSETS_API VertexProperty {
    uint8_t offset;
    uint8_t elementCount;
    VertexPropertyFormat format;
    VertexUsageName usage;

    uint32_t byteCount() const;
};

struct ASSETS_API VertexFormat {
    Core::HashMap<VertexUsageName, VertexProperty> properties;

    uint32_t elementCount() const;
    uint32_t byteCount() const;
};
}    // namespace Assets