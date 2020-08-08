load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

def bengine_cc_binary(name, copts = None, **kwargs):
    cc_binary(
        name = name,
        copts = ["-std:c++latest", "-Zc:__cplusplus"] + (copts or []),
        **kwargs
    )

def bengine_cc_library(name, copts = None, **kwargs):
    cc_library(
        name = name,
        copts = ["-std:c++latest", "-Zc:__cplusplus"] + (copts or []),
        **kwargs
    )
