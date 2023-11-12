load("//tools:bengine_rules.bzl", "bengine_cc_binary", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_binary(
    name = "shader_compiler",
    srcs = ["ShaderCompiler.cpp"],
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
        "@nlohmann-json",
    ],
)
