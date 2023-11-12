load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "window",
    srcs = ["Window.cpp"],
    hdrs = ["Window.h"],
    deps = [
        "//core/assert",
        "//core/containers:hash_set",
        "//core/logging",
        "//core/status",
        "//renderer/backends/vulkan",
        "@glfw",
    ],
)
