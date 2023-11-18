load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

def bengine_cc_binary(name, copts = None, **kwargs):
    cc_binary(
        name = name,
        copts = ["-std:c++latest", "-Zc:__cplusplus", "-permissive-"] + (copts or []),
        **kwargs
    )

def bengine_cc_library(name, copts = None, **kwargs):
    cc_library(
        name = name,
        copts = ["-std:c++latest", "-Zc:__cplusplus", "-permissive-"] + (copts or []),
        **kwargs
    )

def bengine_cc_test(name, deps = None, copts = None, **kwargs):
    cc_test(
        name = name,
        deps = ["//tools:catch2_test_main"] + (deps or []),
        copts = ["-std:c++latest", "-Zc:__cplusplus", "-permissive-"] + (copts or []),
        **kwargs
    )
