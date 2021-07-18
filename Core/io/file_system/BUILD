load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "file_system",
    srcs = [
        "src/BareFileSystemMount.cpp",
        "src/FileSystem.cpp",
        "src/FileSystemMount.cpp",
        "src/Path.cpp",
        "src/VirtualFileSystemMount.cpp",
    ],
    hdrs = [
        "include/Core/IO/FileSystem/BareFileSystemMount.h",
        "include/Core/IO/FileSystem/FileSystem.h",
        "include/Core/IO/FileSystem/FileSystemMount.h",
        "include/Core/IO/FileSystem/Path.h",
        "include/Core/IO/FileSystem/VirtualFileSystemMount.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/algorithms:hashing",
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/containers:hash_set",
        "//core/status",
        "@simple_file_watcher",
    ],
)
