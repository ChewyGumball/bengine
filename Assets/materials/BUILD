load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "materials",
    hdrs = [
        "Material.h",
        "Shader.h",
    ],
    deps = [
        "//assets/buffers:buffer_layout",
        "//assets/models",
        "//core/containers:hash_map",
    ],
)
