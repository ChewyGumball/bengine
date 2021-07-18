load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "concepts",
    hdrs = ["include/Core/IO/Serialization/BinarySerializable.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "streams",
    srcs = [
        "src/InputStream.cpp",
        "src/OutputStream.cpp",
    ],
    hdrs = [
        "include/Core/IO/Serialization/InputStream.h",
        "include/Core/IO/Serialization/OutputStream.h",
    ],
    strip_include_prefix = "include",
    deps = [":concepts"],
)

bengine_cc_library(
    name = "buffers",
    srcs = [
        "src/ArrayBuffer.cpp",
        "src/BufferView.cpp",
        "src/MemoryBuffer.cpp",
    ],
    hdrs = [
        "include/Core/IO/Serialization/ArrayBuffer.h",
        "include/Core/IO/Serialization/BufferView.h",
        "include/Core/IO/Serialization/MemoryBuffer.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":concepts",
        "//core/assert",
        "//core/containers:array",
        "//core/status",
    ],
)

bengine_cc_test(
    name = "test_output_stream",
    srcs = [
        "test/test_output_stream.cpp",
    ],
    deps = [
        ":buffers",
        ":streams",
    ],
)
