load("@bengine//tools:bengine_rules.bzl", "bengine_cc_library")

bengine_cc_library(
    name = "glfw",
    srcs = [
        # Windows specific files
        "src/win32_init.c",
        "src/win32_joystick.c",
        "src/win32_joystick.h",
        "src/win32_monitor.c",
        "src/win32_platform.h",
        "src/win32_thread.c",
        "src/win32_time.c",
        "src/win32_window.c",
        "src/wgl_context.h",
        "src/wgl_context.c",

        # Platform independent files
        "src/window.c",
        "src/vulkan.c",
        "src/monitor.c",
        "src/mappings.h",
        "src/internal.h",
        "src/input.c",
        "src/init.c",
        "src/context.c",
        "src/egl_context.h",
        "src/egl_context.c",
        "src/osmesa_context.h",
        "src/osmesa_context.c",
        "src/xkb_unicode.h",
        "src/xkb_unicode.c",
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",

        # Headless files
        # "src/null_init.c",
        # "src/null_joystick.c",
        # "src/null_joystick.h",
        # "src/null_monitor.c",
        # "src/null_platform.h",
        # "src/null_window.c",
    ],
    hdrs = [
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
    ],
    linkopts = [
        "-DEFAULTLIB:user32.lib",
        "-DEFAULTLIB:gdi32.lib",
        "-DEFAULTLIB:shell32.lib",
    ],
    local_defines = ["_GLFW_WIN32 "],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
