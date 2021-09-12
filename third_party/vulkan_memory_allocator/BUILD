load("@bengine//tools:bengine_rules.bzl", "bengine_cc_library")

bengine_cc_library(
    name = "vulkan_memory_allocator",
    hdrs = ["src/vk_mem_alloc.h"],
    include_prefix = "VMA",
    strip_include_prefix = "src",
    visibility = ["//visibility:public"],
)

filegroup(
    name = "natvis",
    srcs = ["src/vk_mem_alloc.natvis"],
    visibility = ["//visibility:public"],
)
