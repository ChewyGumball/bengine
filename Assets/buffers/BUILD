load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "buffer_layout",
    srcs = ["BufferLayout.cpp"],
    hdrs = ["BufferLayout.h"],
    deps = [
        "//core/algorithms:hashing",
        "//core/containers:hash_map",
        "//core/io/serialization:streams",
    ],
)
