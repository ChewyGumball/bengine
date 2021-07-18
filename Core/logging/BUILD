load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "logging",
    srcs = ["src/Logger.cpp"],
    hdrs = [
        "include/Core/Logging/LogCategory.h",
        "include/Core/Logging/Logger.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "@fmt",
        "@spdlog",
    ],
)
