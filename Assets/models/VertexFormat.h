#pragma once

#include "assets/buffers/BufferLayout.h"

#include "core/containers/HashMap.h"
#include "core/io/serialization/InputStream.h"
#include "core/io/serialization/OutputStream.h"

namespace Assets {

using VertexUsageName = uint32_t;

namespace VertexUsage {
constexpr VertexUsageName POSITION  = 0;
constexpr VertexUsageName NORMAL    = 1;
constexpr VertexUsageName COLOUR    = 2;
constexpr VertexUsageName TEXTURE   = 3;
constexpr VertexUsageName TEXTURE2  = 4;
constexpr VertexUsageName TRANSFORM = 5;

constexpr std::string_view AsString(const VertexUsageName usage) {
    switch(usage) {
        case POSITION: return "Position";
        case NORMAL: return "Normal";
        case COLOUR: return "Colour";
        case TEXTURE: return "Texture";
        case TEXTURE2: return "Texture2";
        default: return "Unknown Property";
    }
}
}    // namespace VertexUsage

using VertexFormat = BufferLayout<VertexUsageName>;
}    // namespace Assets
