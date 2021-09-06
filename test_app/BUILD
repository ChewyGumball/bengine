load("//tools:bengine_rules.bzl", "bengine_cc_binary")

bengine_cc_binary(
    name = "test_app",
    srcs = [
        "Test.cpp",
    ],
    data = [
        "//content:chalet",
        "//content:chalet_texture",
        "//content:triangle",
    ],
    deps = [
        "//assets/materials",
        "//assets/models",
        "//assets/textures",
        "//core/io/serialization:buffers",
        "//core/time",
        "//gui:window",
        "//renderer:vulkan_backend",
        "@com_google_absl//absl/container:flat_hash_map",
        "@glfw",
        "@glm",
        "@imgui",
        "@imgui//:imgui_glfw",
        "@imgui//:imgui_vulkan",
    ],
)
