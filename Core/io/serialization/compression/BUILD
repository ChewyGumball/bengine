load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "compression",
    srcs = [
        "Compression.cpp",
        "ZStdCompression.cpp",
        "ZStdCompressionBuffer.cpp",
        "ZStdDecompressionBuffer.cpp",
    ],
    hdrs = [
        "Compression.h",
        "StreamingCompressionBuffer.h",
        "StreamingDecompressionBuffer.h",
        "ZStdCompression.h",
        "ZStdCompressionBuffer.h",
        "ZStdDecompressionBuffer.h",
    ],
    deps = [
        "//core/containers:array",
        "//core/io/serialization:buffers",
        "//core/io/serialization:streams",
        "@zlib",
        "@zstd",
    ],
)
