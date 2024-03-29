#include <catch2/catch_test_macros.hpp>

#include "core/io/serialization/ArrayBuffer.h"
#include "core/io/serialization/OutputStream.h"

#include "assets/models/VertexFormat.h"

TEST_CASE("Vertex Format Serialization", "Serialization") {
    using namespace Assets;

    Core::IO::ArrayBuffer storage;
    Core::IO::OutputStream outStream(&storage);

    VertexFormat format{
          .properties = {
                {VertexUsage::POSITION,
                 BufferProperty{
                       .property = {.type = PropertyType::FLOAT_32, .elementCount = 3}, .byteOffset = 0, .count = 1}},
                {VertexUsage::TEXTURE,
                 BufferProperty{.property   = {.type = PropertyType::FLOAT_32, .elementCount = 2},
                                .byteOffset = 3 * sizeof(float),
                                .count      = 2}},
          }};

    outStream.write(format);

    Core::IO::InputStream inStream(&storage);

    VertexFormat readFormat = inStream.read<VertexFormat>();

    REQUIRE(readFormat.properties.size() == format.properties.size());

    for(auto& [usage, property] : format.properties) {
        REQUIRE(readFormat.properties.contains(usage));
        BufferProperty& readProperty = readFormat.properties[usage];

        CHECK(readProperty.property.type == property.property.type);
        CHECK(readProperty.byteOffset == property.byteOffset);
        CHECK(readProperty.property.elementCount == property.property.elementCount);
    }
}