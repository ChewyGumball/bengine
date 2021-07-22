load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "containers",
    hdrs = ["include/Core/Algorithms/Containers.h"],
    strip_include_prefix = "include",
    deps = [
        "//core/containers:array",
    ],
)

bengine_cc_library(
    name = "hashing",
    hdrs = ["include/Core/Algorithms/Hashing.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "mappers",
    hdrs = ["include/Core/Algorithms/Mappers.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "optional",
    hdrs = ["include/Core/Algorithms/Optional.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "strings",
    srcs = ["src/Strings.cpp"],
    hdrs = ["include/Core/Algorithms/Strings.h"],
    strip_include_prefix = "include",
    deps = [
        "//core/containers:array",
    ],
)
