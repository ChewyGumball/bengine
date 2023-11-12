load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "logging",
    srcs = ["Logger.cpp"],
    hdrs = [
        "LogCategory.h",
        "Logger.h",
    ],
    deps = [
        "@fmt",
        "@spdlog",
    ],
)
