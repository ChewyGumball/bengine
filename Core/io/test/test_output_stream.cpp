#include <catch2/catch_test_macros.hpp>

#include <Core/IO/Serialization/ArrayBuffer.h>
#include <Core/IO/Serialization/InputStream.h>
#include <Core/IO/Serialization/OutputStream.h>


TEST_CASE("Output integers") {
    Core::IO::ArrayBuffer storage;

    Core::IO::OutputStream stream(&storage);

    size_t value = 2;
    stream.write(value);

    const std::byte* rawBuffer = storage.buffer().rawData();
    const std::byte* rawValue  = reinterpret_cast<const std::byte*>(&value);
    for(size_t i = 0; i < sizeof(size_t); i++) {
        CHECK(rawBuffer[i] == rawValue[i]);
    }
}