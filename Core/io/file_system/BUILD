load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "file_system",
    srcs = [
        "BareFileSystemMount.cpp",
        "FileSystem.cpp",
        "FileSystemMount.cpp",
        "Path.cpp",
        "VirtualFileSystemMount.cpp",
    ],
    hdrs = [
        "BareFileSystemMount.h",
        "FileSystem.h",
        "FileSystemMount.h",
        "Path.h",
        "VirtualFileSystemMount.h",
    ],
    deps = [
        "//core/algorithms:hashing",
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/containers:hash_set",
        "//core/status",
        "@simple_file_watcher",
    ],
)
