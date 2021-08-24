load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "buffer_layout",
    srcs = ["src/BufferLayout.cpp"],
    hdrs = ["include/Assets/Buffers/BufferLayout.h"],
    strip_include_prefix = "include",
    deps = [
        "//core/algorithms:hashing",
        "//core/containers:hash_map",
        "//core/io/serialization:streams",
    ],
)
