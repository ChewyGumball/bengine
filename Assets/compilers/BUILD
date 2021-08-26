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
        "@cli11",
    ],
)

bengine_cc_binary(
    name = "shader_compiler",
    srcs = [
        "src/ShaderCompiler.cpp",
    ],
    deps = [
        "//assets/materials",
        "//core/algorithms:strings",
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/containers:span",
        "//core/io/file_system",
        "//core/io/serialization:buffers",
        "//core/io/serialization:streams",
        "@cli11",
        "@glslang",
        "@glslang//:SPIRV",
        "@glslang//:glslang-default-resource-limits",
    ],
)
