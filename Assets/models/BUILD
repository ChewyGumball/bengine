load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "models",
    srcs = ["Mesh.cpp"],
    hdrs = [
        "Mesh.h",
        "Model.h",
        "VertexFormat.h",
    ],
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
