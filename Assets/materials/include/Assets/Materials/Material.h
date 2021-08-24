#pragma once

#include <Core/Containers/HashMap.h>

namespace Assets {

using MaterialInputTypeName = uint32_t;

namespace MaterialInputType {
    constexpr MaterialInputTypeName FLOAT32      = 0;
    constexpr MaterialInputTypeName INT32        = 1;
    constexpr MaterialInputTypeName UINT32       = 2;
    constexpr MaterialInputTypeName FLOAT16      = 3;
    constexpr MaterialInputTypeName INT16        = 4;
    constexpr MaterialInputTypeName UINT16       = 5;
    constexpr MaterialInputTypeName FLOAT32_VEC2 = 6;
    constexpr MaterialInputTypeName INT32_VEC2   = 7;
    constexpr MaterialInputTypeName UINT32_VEC2  = 8;
    constexpr MaterialInputTypeName FLOAT16_VEC2 = 9;
    constexpr MaterialInputTypeName INT16_VEC2   = 10;
    constexpr MaterialInputTypeName UINT16_VEC2  = 11;
    constexpr MaterialInputTypeName FLOAT32_VEC3 = 12;
    constexpr MaterialInputTypeName INT32_VEC3   = 13;
    constexpr MaterialInputTypeName UINT32_VEC3  = 14;
    constexpr MaterialInputTypeName FLOAT16_VEC3 = 15;
    constexpr MaterialInputTypeName INT16_VEC3   = 16;
    constexpr MaterialInputTypeName UINT16_VEC3  = 17;
    constexpr MaterialInputTypeName FLOAT32_VEC4 = 18;
    constexpr MaterialInputTypeName INT32_VEC4   = 19;
    constexpr MaterialInputTypeName UINT32_VEC4  = 20;
    constexpr MaterialInputTypeName FLOAT16_VEC4 = 21;
    constexpr MaterialInputTypeName INT16_VEC4   = 22;
    constexpr MaterialInputTypeName UINT16_VEC4  = 23;
    constexpr MaterialInputTypeName STRUCT       = 24;
    constexpr MaterialInputTypeName TEXTURE      = 25;
    constexpr MaterialInputTypeName SAMPLER      = 26;

    constexpr std::string_view AsString(const MaterialInputTypeName& type) {
        switch(type) {
            case FLOAT32: return "32 bit float";
            case INT32: return "32 bit int";
            case UINT32: return "32 bit uint";
            case FLOAT16: return "16 bit float";
            case INT16: return "16 bit int";
            case UINT16: return "16 bit uint";
            default: return "unknown type";
        }
    }
}    // namespace MaterialInputType

struct MaterialInput {
    uint32_t byteOffset;
    MaterialInputTypeName type;
};
struct Material {
    Core::HashMap<std::string, MaterialInput> inputs;
};
}    // namespace Assets