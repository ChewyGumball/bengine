load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "assert",
    srcs = ["Assert.cpp"],
    hdrs = ["Assert.h"],
    deps = [
        "//core/logging",
        "@boost//:stacktrace",
    ],
)
