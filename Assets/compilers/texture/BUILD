load("//tools:bengine_rules.bzl", "bengine_cc_binary", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "texture_compiler_lib",
    srcs = ["TextureCompiler.cpp"],
    hdrs = ["TextureCompiler.h"],
    deps = [
        "//assets/textures",
        "//core/io/file_system",
        "@stb//:stb_image",
    ],
)

bengine_cc_binary(
    name = "texture_compiler",
    srcs = ["TextureCompilerCLI.cpp"],
    deps = [
        ":texture_compiler_lib",
        "@cli11",
    ],
)
