load("//tools:bengine_rules.bzl", "bengine_cc_binary")

bengine_cc_binary(
    name = "test_app",
    srcs = [
        "Test.cpp",
        "imgui_impl_glfw.cpp",
        "imgui_impl_glfw.h",
        "imgui_impl_vulkan.cpp",
        "imgui_impl_vulkan.h",
    ],
    data = [
        "//content:models",
        "//content:shaders",
        "//content:textures",
    ],
    deps = [
        "//assets:importers",
        "//core:time",
        "//gui:window",
        "//renderer:vulkan",
        "@com_google_absl//absl/container:flat_hash_map",
        "@glfw",
        "@glm",
        "@imgui",
        "@stb//:stb_image",
    ],
)
