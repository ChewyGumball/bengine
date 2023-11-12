load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "status",
    hdrs = [
        "Status.h",
        "StatusOr.h",
    ],
    deps = [
        "//core/assert",
        "@spdlog",
    ],
)
