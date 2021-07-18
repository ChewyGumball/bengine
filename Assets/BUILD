load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "core",
    hdrs = [
        "include/AssetsCore.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/logging",
    ],
)

bengine_cc_library(
    name = "catalogs",
    srcs = [],
    hdrs = [
        "include/Assets/Catalogs/AssetCatalog.h",
        "include/Assets/Catalogs/AssetTag.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/containers:opaque_id",
        "//core/io/file_system",
        "//core/io/serialization:streams",
    ],
)

bengine_cc_library(
    name = "importers",
    srcs = [
        "src/Importers/OBJImporter.cpp",
    ],
    hdrs = [
        "include/Assets/Importers/OBJImporter.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":core",
        ":model",
        "//core/algorithms:strings",
        "//core/containers:array",
        "//core/io/file_system",
    ],
)

bengine_cc_library(
    name = "material",
    hdrs = [
        "include/Assets/Material/Material.h",
        "include/Assets/Material/Shader.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/containers:hash_map",
    ],
)

bengine_cc_library(
    name = "model",
    srcs = [
        "src/Model/Mesh.cpp",
        "src/Model/VertexFormat.cpp",
    ],
    hdrs = [
        "include/Assets/Model/Mesh.h",
        "include/Assets/Model/Model.h",
        "include/Assets/Model/VertexFormat.h",
    ],
    strip_include_prefix = "include",
    deps = [
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
        "//assets:model",
        "//core/io/serialization:buffers",
        "//core/io/serialization:streams",
    ],
)
