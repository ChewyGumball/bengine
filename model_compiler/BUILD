load("//tools:bengine_rules.bzl", "bengine_cc_binary")

bengine_cc_binary(
    name = "compile",
    srcs = [
        "ModelCompiler.cpp",
    ],
    deps = [
        "//assets:importers",
        "//core/assert",
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/io/file_system",
        "//core/io/serialization:buffers",
        "//core/io/serialization:streams",
    ],
)
