load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "time",
    srcs = [
        "src/Clock.cpp",
        "src/SystemClockTicker.cpp",
        "src/Timer.cpp",
    ],
    hdrs = [
        "include/Core/Time/Clock.h",
        "include/Core/Time/SystemClockTicker.h",
        "include/Core/Time/Timer.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/containers:array",
    ],
)
