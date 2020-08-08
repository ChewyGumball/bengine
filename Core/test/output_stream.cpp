#include <catch2/catch.hpp>

#include <Core/IO/ArrayBuffer.h>
#include <Core/IO/InputStream.h>
#include <Core/IO/OutputStream.h>


TEST_CASE("Output integers") {
    Core::IO::ArrayBuffer storage;

    Core::IO::OutputStream stream(&storage);

    size_t value = 2;
    stream.write(value);

    const std::byte* rawBuffer = storage.buffer().data();
    const std::byte* rawValue  = reinterpret_cast<const std::byte*>(&value);
    for(size_t i = 0; i < sizeof(size_t); i++) {
        CHECK(rawBuffer[i] == rawValue[i]);
    }
}