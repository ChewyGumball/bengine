load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "window",
    srcs = [
        "src/Window.cpp",
    ],
    hdrs = ["include/GUI/Window.h"],
    strip_include_prefix = "include",
    deps = [
        "//core/assert",
        "//core/containers:hash_set",
        "//core/logging",
        "//core/status",
        "//renderer:vulkan",
        "@glfw",
    ],
)
