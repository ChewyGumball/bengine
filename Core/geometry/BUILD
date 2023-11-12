load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "geometry",
    srcs = [
        "Plane.cpp",
        "Transform.cpp",
        "ViewFrustum.cpp",
    ],
    hdrs = [
        "BoundingBox.h",
        "Plane.h",
        "Transform.h",
        "ViewFrustum.h",
    ],
    deps = [
        "//core/algorithms:containers",
        "//core/containers:array",
        "@glm",
    ],
)
