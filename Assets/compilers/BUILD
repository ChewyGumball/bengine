load("//tools:bengine_rules.bzl", "bengine_cc_binary", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "model_compiler_lib",
    srcs = [
        "src/ModelCompiler.cpp",
    ],
    hdrs = [
        "include/Assets/Compilers/ModelCompiler.h",
    ],
    strip_include_prefix = "include",
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

bengine_cc_binary(
    name = "model_compiler",
    srcs = [
        "src/ModelCompilerCLI.cpp",
    ],
    deps = [
        ":model_compiler_lib",
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
        "@nlohmann-json",
    ],
)

bengine_cc_library(
    name = "texture_compiler_lib",
    srcs = ["src/TextureCompiler.cpp"],
    hdrs = ["include/Assets/Compilers/TextureCompiler.h"],
    strip_include_prefix = "include",
    deps = [
        "//assets/textures",
        "@stb//:stb_image",
        "//core/io/file_system",
    ]
)

bengine_cc_binary(
    name = "texture_compiler",
    srcs = [
        "src/TextureCompilerCLI.cpp",
    ],
    deps = [
        ":texture_compiler_lib",
        "@cli11",
    ]
)