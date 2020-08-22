load("@bengine//tools:bengine_rules.bzl", "bengine_cc_library")

bengine_cc_library(
    name = "fmt",
    srcs = [
        "src/format.cc",
        "src/os.cc",
    ],
    hdrs = [
        "include/fmt/chrono.h",
        "include/fmt/color.h",
        "include/fmt/compile.h",
        "include/fmt/core.h",
        "include/fmt/format.h",
        "include/fmt/format-inl.h",
        "include/fmt/locale.h",
        "include/fmt/os.h",
        "include/fmt/ostream.h",
        "include/fmt/posix.h",
        "include/fmt/printf.h",
        "include/fmt/ranges.h",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
