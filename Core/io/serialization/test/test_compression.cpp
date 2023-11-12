#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "core/io/serialization/compression/Compression.h"


TEST_CASE("ZStd") {
    using namespace Core::IO::Compression;

    Core::Array<std::byte> data;
    for(uint8_t i = 0; i < 100; i++) {
        data.emplace(std::byte(i));
    }

    Core::StatusOr<Core::Array<std::byte>> compressedStatus =
          Compress(data, CompressionFlags{.format = CompressionFormat::ZSTD});
    if(compressedStatus.isError()) {
        FAIL(compressedStatus.message());
    }

    Core::Array<std::byte> compressedData = std::move(compressedStatus).value();


    Core::StatusOr<Core::Array<std::byte>> decompressedStatus =
          Decompress(compressedData, data.count(), CompressionFlags{.format = CompressionFormat::ZSTD});
    if(decompressedStatus.isError()) {
        FAIL(decompressedStatus.message());
    }

    Core::Array<std::byte> decompressedData = std::move(decompressedStatus).value();

    CHECK_THAT(decompressedData, Catch::Matchers::RangeEquals(data));
}