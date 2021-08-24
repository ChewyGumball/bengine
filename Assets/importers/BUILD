load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "obj_importer",
    srcs = [
        "src/OBJImporter.cpp",
    ],
    hdrs = [
        "include/Assets/Importers/OBJImporter.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//assets/models",
        "//core/algorithms:strings",
        "//core/containers:array",
        "//core/io/file_system",
    ],
)
