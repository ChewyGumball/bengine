load("//tools:bengine_rules.bzl", "bengine_cc_binary")

bengine_cc_binary(
    name = "test",
    srcs = [
        "Test.cpp",
        "Test.h",
        "imgui_impl_glfw.cpp",
        "imgui_impl_glfw.h",
        "imgui_impl_vulkan.cpp",
        "imgui_impl_vulkan.h",
    ],
    data = [
        "Content/Models/chalet.mesh",
        "Content/Shaders/triangle/frag.spv",
        "Content/Shaders/triangle/vert.spv",
        "Content/Textures/chalet.jpg",
    ],
    deps = [
        "//assets:importers",
        "//core:time",
        "//renderer:vulkan",
        "@com_google_absl//absl/container:flat_hash_map",
        "@glfw",
        "@glm",
        "@imgui",
        "@stb//:stb_image",
    ],
)
