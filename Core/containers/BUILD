load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "array",
    hdrs = [
        "Array.h",
        "internal/Array.inl",
    ],
    deps = [
        ":span",
        "//core/algorithms:memory",
        "//core/assert",
        "//core/io/serialization:streams",
    ],
)

bengine_cc_library(
    name = "hash_map",
    hdrs = ["HashMap.h"],
    deps = ["//core/io/serialization:streams"],
)

bengine_cc_library(
    name = "hash_set",
    hdrs = ["HashSet.h"],
)

bengine_cc_library(
    name = "opaque_id",
    hdrs = ["OpaqueID.h"],
)

bengine_cc_library(
    name = "ordered_map",
    hdrs = ["OrderedMap.h"],
)

bengine_cc_library(
    name = "queue",
    hdrs = ["Queue.h"],
)

bengine_cc_library(
    name = "span",
    hdrs = [
        "Span.h",
        "internal/Span.inl",
    ],
)

bengine_cc_library(
    name = "index_span",
    hdrs = ["IndexSpan.h"],
    deps = [
        "//core/assert",
        "//core/io/serialization:streams",
    ],
)

bengine_cc_library(
    name = "visitor",
    hdrs = ["Visitor.h"],
)

bengine_cc_test(
    name = "test_array",
    srcs = ["test/test_array.cpp"],
    shard_count = 5,
    deps = [
        ":array",
        "//core/io/serialization:buffers",
    ],
)
