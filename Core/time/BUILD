load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "time",
    srcs = [
        "Clock.cpp",
        "SystemClockTicker.cpp",
        "SystemTimer.cpp",
        "Timer.cpp",
    ],
    hdrs = [
        "Clock.h",
        "SystemClockTicker.h",
        "SystemTimer.h",
        "Timer.h",
    ],
    deps = [
        "//core/containers:array",
    ],
)
