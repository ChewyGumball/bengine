load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "catalogs",
    srcs = [],
    hdrs = [
        "AssetCatalog.h",
        "AssetReference.h",
        "TextureCatalog.h",
    ],
    deps = [
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/containers:opaque_id",
        "//core/io/file_system",
        "//core/io/serialization:streams",
    ],
)
