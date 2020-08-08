load("@bengine//tools:bengine_rules.bzl", "bengine_cc_library")

bengine_cc_library(
    name = "simple_file_watcher",
    srcs = [
        "source/FileWatcher.cpp",
        "source/FileWatcherLinux.cpp",
        "source/FileWatcherOSX.cpp",
        "source/FileWatcherWin32.cpp",
    ],
    hdrs = [
        "include/FileWatcher/FileWatcher.h",
        "include/FileWatcher/FileWatcherImpl.h",
        "include/FileWatcher/FileWatcherLinux.h",
        "include/FileWatcher/FileWatcherOSX.h",
        "include/FileWatcher/FileWatcherWin32.h",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
