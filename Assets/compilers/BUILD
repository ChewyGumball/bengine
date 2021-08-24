load("//tools:bengine_rules.bzl", "bengine_cc_binary")

package(default_visibility = ["//visibility:public"])

bengine_cc_binary(
    name = "model_compiler",
    srcs = [
        "src/ModelCompiler.cpp",
    ],
    deps = [
        "//assets/importers:obj_importer",
        "//core/assert",
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/io/file_system",
        "//core/io/serialization:buffers",
        "//core/io/serialization:streams",
    ],
)
