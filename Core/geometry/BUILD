load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "geometry",
    srcs = [
        "src/Plane.cpp",
        "src/Transform.cpp",
        "src/ViewFrustum.cpp",
    ],
    hdrs = [
        "include/Core/Geometry/BoundingBox.h",
        "include/Core/Geometry/Plane.h",
        "include/Core/Geometry/Transform.h",
        "include/Core/Geometry/ViewFrustum.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/algorithms:containers",
        "//core/containers:array",
        "@glm",
    ],
)
