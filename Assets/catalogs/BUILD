load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "catalogs",
    srcs = [],
    hdrs = [
        "include/Assets/Catalogs/AssetCatalog.h",
        "include/Assets/Catalogs/AssetReference.h",
        "include/Assets/Catalogs/TextureCatalog.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/containers:array",
        "//core/containers:hash_map",
        "//core/containers:opaque_id",
        "//core/io/file_system",
        "//core/io/serialization:streams",
    ],
)
