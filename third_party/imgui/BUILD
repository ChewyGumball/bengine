load("@bengine//tools:bengine_rules.bzl", "bengine_cc_library")

bengine_cc_library(
    name = "imgui",
    srcs = [
        "imconfig.h",
        "imgui.cpp",
        "imgui.h",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_internal.h",
        "imgui_widgets.cpp",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
    ],
    hdrs = [
        "imgui.h",
    ],
    include_prefix = "imgui",
    visibility = ["//visibility:public"],
)

bengine_cc_library(
    name = "imgui_vulkan",
    srcs = [
        "examples/imgui_impl_vulkan.cpp",
        "examples/imgui_impl_vulkan.h",
    ],
    hdrs = [
        "examples/imgui_impl_vulkan.h",
    ],
    include_prefix = "imgui",
    strip_include_prefix = "examples",
    visibility = ["//visibility:public"],
    deps = [
        ":imgui",
        "@vulkan_sdk_windows//:vulkan",
    ],
)

bengine_cc_library(
    name = "imgui_glfw",
    srcs = [
        "examples/imgui_impl_glfw.cpp",
        "examples/imgui_impl_glfw.h",
    ],
    hdrs = [
        "examples/imgui_impl_glfw.h",
    ],
    include_prefix = "imgui",
    strip_include_prefix = "examples",
    visibility = ["//visibility:public"],
    deps = [
        ":imgui",
        "@glfw",
    ],
)
