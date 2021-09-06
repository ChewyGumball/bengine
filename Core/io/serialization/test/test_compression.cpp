#include <catch2/catch_test_macros.hpp>

#include <Core/IO/Serialization/ArrayBuffer.h>
#include <Core/IO/Serialization/OutputStream.h>

#include <Core/IO/Serialization/Compression.h>


TEST_CASE("ZLib Header") {
    Core::IO::ArrayBuffer storage;

    Core::IO::OutputStream stream(&storage);

    Core::Array<std::byte> data;
    for(uint8_t i = 0; i < 100; i++) {
        data.emplace(std::byte(i));
    }

    REQUIRE(Core::IO::ZLibCompressToStream(data, stream).isOk());

    CHECK(storage.buffer()[0] == std::byte(0x78));
    CHECK(storage.buffer()[1] == std::byte(0x9c));
}