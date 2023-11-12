load("//tools:bengine_rules.bzl", "bengine_cc_binary", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "model_compiler_lib",
    srcs = ["ModelCompiler.cpp"],
    hdrs = ["ModelCompiler.h"],
    deps = [
        "//assets/importers/obj:obj_importer",
        "//core/assert",
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/io/file_system",
        "//core/io/serialization:buffers",
        "//core/io/serialization:streams",
    ],
)

bengine_cc_binary(
    name = "model_compiler",
    srcs = ["ModelCompilerCLI.cpp"],
    deps = [
        ":model_compiler_lib",
        "@cli11",
    ],
)
