load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "assert",
    srcs = [
        "src/Assert.cpp",
    ],
    hdrs = ["include/Core/Assert.h"],
    strip_include_prefix = "include",
    deps = [
        "//core/logging",
        "@boost//:stacktrace",
    ],
)
