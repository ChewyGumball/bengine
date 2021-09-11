load("//tools:bengine_rules.bzl", "bengine_cc_binary")
load("//debug:debug.bzl", "combine_natviz_files")

bengine_cc_binary(
    name = "natviz_combiner",
    srcs = ["NatvizCombiner.cpp"],
    deps = [
        "//core/algorithms:strings",
        "//core/io/file_system",
        "//core/io/serialization:streams",
        "//core/logging",
        "@cli11",
    ],
)

combine_natviz_files(
    name = "debug",
    srcs = [
        "containers.natvis",
        "@vulkan_memory_allocator//:natvis",
    ],
)
