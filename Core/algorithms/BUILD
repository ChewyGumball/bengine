load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "containers",
    hdrs = ["Containers.h"],
    deps = ["//core/containers:array"],
)

bengine_cc_library(
    name = "hashing",
    hdrs = ["Hashing.h"],
)

bengine_cc_library(
    name = "mappers",
    hdrs = ["Mappers.h"],
)

bengine_cc_library(
    name = "optional",
    hdrs = ["Optional.h"],
)

bengine_cc_library(
    name = "strings",
    srcs = ["Strings.cpp"],
    hdrs = ["Strings.h"],
    deps = ["//core/containers:array"],
)

bengine_cc_library(
    name = "memory",
    hdrs = ["Memory.h"],
    deps = ["//core/assert"],
)
