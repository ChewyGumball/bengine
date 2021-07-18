load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "status",
    hdrs = [
        "include/Core/Status/Status.h",
        "include/Core/Status/StatusOr.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/assert",
        "@spdlog",
    ],
)
