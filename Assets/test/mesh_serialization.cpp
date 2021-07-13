#include <catch2/catch_test_macros.hpp>

#include <Core/IO/ArrayBuffer.h>
#include <Core/IO/OutputStream.h>

#include <Assets/Model/VertexFormat.h>

TEST_CASE("Vertex Format Serialization", "Serialization") {
    using namespace Assets;

    Core::IO::ArrayBuffer storage;
    Core::IO::OutputStream outStream(&storage);

    VertexFormat format{.properties = {
                              {VertexUsage::POSITION,
                               VertexProperty{.format       = VertexPropertyFormat::FLOAT_32,
                                              .usage        = VertexUsage::POSITION,
                                              .byteOffset   = 0,
                                              .elementCount = 3}},
                              {VertexUsage::TEXTURE,
                               VertexProperty{.format       = VertexPropertyFormat::FLOAT_32,
                                              .usage        = VertexUsage::TEXTURE,
                                              .byteOffset   = 3 * sizeof(float),
                                              .elementCount = 2}},
                        }};

    outStream.write(format);

    Core::IO::InputStream inStream(&storage);

    VertexFormat readFormat = inStream.read<VertexFormat>();

    REQUIRE(readFormat.properties.size() == format.properties.size());

    for(auto& [usage, property] : format.properties) {
        REQUIRE(readFormat.properties.contains(usage));
        VertexProperty& readProperty = readFormat.properties[usage];

        CHECK(readProperty.format == property.format);
        CHECK(readProperty.usage == property.usage);
        CHECK(readProperty.byteOffset == property.byteOffset);
        CHECK(readProperty.elementCount == property.elementCount);
    }
}