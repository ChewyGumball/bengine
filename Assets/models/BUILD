load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "models",
    srcs = [
        "src/Mesh.cpp",
    ],
    hdrs = [
        "include/Assets/Models/Mesh.h",
        "include/Assets/Models/Model.h",
        "include/Assets/Models/VertexFormat.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//assets/buffers:buffer_layout",
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/containers:index_span",
        "//core/logging",
    ],
)

bengine_cc_test(
    name = "mesh_serialization",
    srcs = ["test/mesh_serialization.cpp"],
    deps = [
        ":models",
        "//core/io/serialization:buffers",
        "//core/io/serialization:streams",
    ],
)
