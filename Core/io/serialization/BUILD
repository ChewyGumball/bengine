load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "concepts",
    hdrs = ["BinarySerializable.h"],
)

bengine_cc_library(
    name = "streams",
    srcs = [
        "InputStream.cpp",
        "OutputStream.cpp",
    ],
    hdrs = [
        "InputStream.h",
        "OutputStream.h",
    ],
    deps = [
        ":concepts",
        "//core/containers:span",
        "//core/status",
    ],
)

bengine_cc_library(
    name = "buffers",
    srcs = [
        "ArrayBuffer.cpp",
        "BufferView.cpp",
        "MemoryBuffer.cpp",
    ],
    hdrs = [
        "ArrayBuffer.h",
        "BufferView.h",
        "MemoryBuffer.h",
    ],
    deps = [
        ":concepts",
        "//core/assert",
        "//core/containers:array",
        "//core/status",
    ],
)

bengine_cc_test(
    name = "test_output_stream",
    srcs = ["test/test_output_stream.cpp"],
    deps = [
        ":buffers",
        ":streams",
    ],
)

bengine_cc_test(
    name = "test_compression",
    srcs = ["test/test_compression.cpp"],
    deps = [
        ":buffers",
        ":compression",
        ":streams",
    ],
)
