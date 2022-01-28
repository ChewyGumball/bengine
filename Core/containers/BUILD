load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "array",
    hdrs = [
        "include/Core/Containers/Array.h",
        "include/Core/Containers/internal/Array.inl",
    ],
    strip_include_prefix = "include",
    deps = [
        ":span",
        "//core/algorithms:memory",
        "//core/assert",
        "//core/io/serialization:streams",
    ],
)

bengine_cc_library(
    name = "hash_map",
    hdrs = ["include/Core/Containers/HashMap.h"],
    strip_include_prefix = "include",
    deps = [
        "//core/io/serialization:streams",
    ],
)

bengine_cc_library(
    name = "hash_set",
    hdrs = ["include/Core/Containers/HashSet.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "opaque_id",
    hdrs = ["include/Core/Containers/OpaqueID.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "ordered_map",
    hdrs = ["include/Core/Containers/OrderedMap.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "queue",
    hdrs = ["include/Core/Containers/Queue.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "span",
    hdrs = ["include/Core/Containers/Span.h"],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "index_span",
    hdrs = ["include/Core/Containers/IndexSpan.h"],
    strip_include_prefix = "include",
    deps = [
        "//core/assert",
        "//core/io/serialization:streams",
    ],
)

bengine_cc_library(
    name = "visitor",
    hdrs = ["include/Core/Containers/Visitor.h"],
    strip_include_prefix = "include",
)

bengine_cc_test(
    name = "test_array",
    srcs = [
        "test/test_array.cpp",
    ],
    shard_count = 5,
    deps = [
        ":array",
        "//core/io/serialization:buffers",
    ],
)
