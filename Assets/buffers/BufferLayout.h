#pragma once

#include <Core/Algorithms/Hashing.h>
#include <Core/Containers/HashMap.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/IO/Serialization/OutputStream.h>

namespace Assets {

using PropertyTypeName = uint32_t;
namespace PropertyType {
constexpr PropertyTypeName FLOAT_32 = 0;
constexpr PropertyTypeName FLOAT_64 = 1;
constexpr PropertyTypeName INT_16   = 2;
constexpr PropertyTypeName INT_32   = 3;
constexpr PropertyTypeName UINT_16  = 4;
constexpr PropertyTypeName UINT_32  = 5;

constexpr std::string_view AsString(const PropertyTypeName propertyType) {
    switch(propertyType) {
        case FLOAT_32: return "32 bit float";
        case FLOAT_64: return "64 bit float";
        case INT_16: return "16 bit int";
        case INT_32: return "32 bit int";
        case UINT_16: return "16 bit uint";
        case UINT_32: return "32 bit uint";
        default: return "unknown property type";
    }
}

constexpr size_t SizeInBytes(const PropertyTypeName propertyType) {
    switch(propertyType) {
        case FLOAT_32: return sizeof(float);
        case FLOAT_64: return sizeof(double);
        case INT_16: return sizeof(int16_t);
        case INT_32: return sizeof(int32_t);
        case UINT_16: return sizeof(uint16_t);
        case UINT_32: return sizeof(uint32_t);
        default: return sizeof(float);
    }
}
}    // namespace PropertyType

struct Property {
    PropertyTypeName type;
    uint8_t elementCount;

    uint8_t byteCount() const;
};

bool operator==(const Property& a, const Property& b);
bool operator!=(const Property& a, const Property& b);

constexpr Property GLSL_ATTRIBUTE_TEMPLATE{
      .type         = PropertyType::FLOAT_32,
      .elementCount = 4,
};

struct BufferProperty {
    Property property;
    uint16_t byteOffset;
    uint8_t count;    // how many values of the type are there (matrices are represented with 4 vec values, rather than
                      // 1 mat value)

    uint8_t byteCount() const {
        return property.byteCount() * count;
    }
};

template <typename INDEX_TYPE>
struct BufferLayout {
    Core::HashMap<INDEX_TYPE, BufferProperty> properties;

    uint32_t byteCount() const {
        uint32_t bytes = 0;
        for(auto& [index, property] : properties) {
            bytes += property.byteCount();
        }
        return bytes;
    }

    bool operator==(const BufferLayout<INDEX_TYPE>& other) {
        return properties == other.properties;
    }

    bool operator!=(const BufferLayout<INDEX_TYPE>& other) {
        return !(*this == other);
    }
};
}    // namespace Assets

namespace Core::IO {
template <typename INDEX_TYPE>
struct Serializer<Assets::BufferLayout<INDEX_TYPE>> {
    static void serialize(Core::IO::OutputStream& stream, const Assets::BufferLayout<INDEX_TYPE>& value) {
        stream.write(value.properties);
    }
};

template <typename INDEX_TYPE>
struct Deserializer<Assets::BufferLayout<INDEX_TYPE>> {
    static Assets::BufferLayout<INDEX_TYPE> deserialize(Core::IO::InputStream& stream) {
        return Assets::BufferLayout<INDEX_TYPE>{.properties =
                                                      stream.read<Core::HashMap<INDEX_TYPE, Assets::BufferProperty>>()};
    }
};
}    // namespace Core::IO

namespace std {
template <>
struct hash<Assets::Property> {
    size_t operator()(const Assets::Property& property) const noexcept {
        return Core::Algorithms::CombineHashes(property.type, property.elementCount);
    }
};
}    // namespace std
