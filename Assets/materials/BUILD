load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "materials",
    hdrs = [
        "include/Assets/Materials/Material.h",
        "include/Assets/Materials/Shader.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//assets/buffers:buffer_layout",
        "//assets/models",
        "//core/containers:hash_map",
    ],
)
