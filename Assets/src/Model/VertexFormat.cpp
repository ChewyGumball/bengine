#include "Assets/Model/VertexFormat.h"

#include <Core/Logging/Logger.h>


namespace {
Core::LogCategory VertexFormatLog("VertexFormat");
}

namespace Assets {

uint32_t VertexProperty::byteCount() const {
    size_t elementSize = 0;
    switch(format) {
        case VertexPropertyFormat::FLOAT_32: elementSize = sizeof(float); break;
        case VertexPropertyFormat::FLOAT_64: elementSize = sizeof(double); break;
        case VertexPropertyFormat::INT_16: elementSize = sizeof(int16_t); break;
        case VertexPropertyFormat::INT_32: elementSize = sizeof(int32_t); break;
        case VertexPropertyFormat::UINT_16: elementSize = sizeof(uint16_t); break;
        case VertexPropertyFormat::UINT_32: elementSize = sizeof(uint32_t); break;
        default:
            elementSize = sizeof(float);
            Core::Log::Error(
                  VertexFormatLog, "Unknown vertex format {}, assuming 32 bit float", format);
    }
    return static_cast<uint32_t>(elementSize * elementCount);
}

uint32_t VertexFormat::byteCount() const {
    uint32_t bytes = 0;
    for(auto& property : properties) {
        bytes += property.second.byteCount();
    }
    return bytes;
}
}    // namespace Assets