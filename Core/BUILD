load("//tools:bengine_rules.bzl", "bengine_cc_library", "bengine_cc_test")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "assert",
    srcs = [
        "src/Assert.cpp",
    ],
    hdrs = ["include/Assert.h"],
    include_prefix = "Core",
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "status",
    hdrs = [
        "include/Core/Status.h",
        "include/Core/StatusOr.h",
    ],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "algorithms",
    srcs = [
        "src/Algorithms/Strings.cpp",
    ],
    hdrs = [
        "include/Core/Algorithms/Containers.h",
        "include/Core/Algorithms/Mappers.h",
        "include/Core/Algorithms/Optional.h",
        "include/Core/Algorithms/Strings.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":containers",
    ],
)

bengine_cc_library(
    name = "containers",
    hdrs = [
        "include/Core/Containers/Array.h",
        "include/Core/Containers/ArrayView.h",
        "include/Core/Containers/HashMap.h",
        "include/Core/Containers/HashSet.h",
        "include/Core/Containers/OrderedMap.h",
        "include/Core/Containers/Queue.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":io_streams",
    ],
)

bengine_cc_library(
    name = "geometry",
    srcs = [
        "src/Geometry/Plane.cpp",
        "src/Geometry/Transform.cpp",
        "src/Geometry/ViewFrustum.cpp",
    ],
    hdrs = [
        "include/Core/Geometry/BoundingBox.h",
        "include/Core/Geometry/Plane.h",
        "include/Core/Geometry/Transform.h",
        "include/Core/Geometry/ViewFrustum.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":algorithms",
        ":containers",
        "@glm",
    ],
)

bengine_cc_library(
    name = "io_streams",
    srcs = [
        "src/IO/InputStream.cpp",
        "src/IO/OutputStream.cpp",
    ],
    hdrs = [
        "include/Core/IO/InputStream.h",
        "include/Core/IO/OutputStream.h",
    ],
    strip_include_prefix = "include",
)

bengine_cc_library(
    name = "io",
    srcs = [
        "src/IO/ArrayBuffer.cpp",
        "src/IO/BufferView.cpp",
        "src/IO/FileSystem/BareFileSystemMount.cpp",
        "src/IO/FileSystem/FileSystem.cpp",
        "src/IO/FileSystem/FileSystemMount.cpp",
        "src/IO/FileSystem/Path.cpp",
        "src/IO/FileSystem/VirtualFileSystemMount.cpp",
        "src/IO/MemoryBuffer.cpp",
    ],
    hdrs = [
        "include/Core/IO/ArrayBuffer.h",
        "include/Core/IO/BufferView.h",
        "include/Core/IO/FileSystem/BareFileSystemMount.h",
        "include/Core/IO/FileSystem/FileSystem.h",
        "include/Core/IO/FileSystem/FileSystemMount.h",
        "include/Core/IO/FileSystem/Path.h",
        "include/Core/IO/FileSystem/VirtualFileSystemMount.h",
        "include/Core/IO/MemoryBuffer.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":containers",
        "@simple_file_watcher",
    ],
)

bengine_cc_library(
    name = "logging",
    srcs = ["src/Logging/Logger.cpp"],
    hdrs = [
        "include/Core/Logging/LogCategory.h",
        "include/Core/Logging/Logger.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":containers",
        "@spdlog",
    ],
)

bengine_cc_library(
    name = "time",
    srcs = [
        "src/Time/Clock.cpp",
        "src/Time/SystemClockTicker.cpp",
        "src/Time/Timer.cpp",
    ],
    hdrs = [
        "include/Core/Time/Clock.h",
        "include/Core/Time/SystemClockTicker.h",
        "include/Core/Time/Timer.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":containers",
    ],
)

bengine_cc_test(
    name = "output_stream",
    srcs = [
        "test/output_stream.cpp",
    ],
    deps = [
        ":io",
    ],
)
