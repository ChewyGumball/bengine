load("//tools:bengine_rules.bzl", "bengine_cc_binary")

bengine_cc_binary(
    name = "test_app",
    srcs = [
        "Test.cpp",
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
        "@imgui//:imgui_glfw",
        "@imgui//:imgui_vulkan",
        "@stb//:stb_image",
    ],
)
