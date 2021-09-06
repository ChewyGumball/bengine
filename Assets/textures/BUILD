load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "textures",
    hdrs = [
        "include/Assets/Textures/Texture.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/containers:array",
        "//core/io/serialization:compression",
        "//core/io/serialization:streams",
        "//core/logging",
    ],
)
