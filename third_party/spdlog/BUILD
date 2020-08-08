load("@bengine//tools:bengine_rules.bzl", "bengine_cc_library")

bengine_cc_library(
    name = "spdlog",
    srcs = [
        "src/async.cpp",
        "src/cfg.cpp",
        "src/color_sinks.cpp",
        "src/file_sinks.cpp",
        "src/fmt.cpp",
        "src/spdlog.cpp",
        "src/stdout_sinks.cpp",
    ],
    hdrs = glob(["include/spdlog/**/*"]),
    local_defines = ["SPDLOG_COMPILED_LIB"],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
