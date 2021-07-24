#pragma once

#include <Assets/BufferLayout.h>

#include <Core/Containers/HashMap.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/IO/Serialization/OutputStream.h>

namespace Assets {

using VertexUsageName = uint32_t;

namespace VertexUsage {
constexpr VertexUsageName POSITION = 0;
constexpr VertexUsageName NORMAL   = 1;
constexpr VertexUsageName COLOUR   = 2;
constexpr VertexUsageName TEXTURE  = 3;
constexpr VertexUsageName TEXTURE2 = 4;

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
