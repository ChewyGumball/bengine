workspace(name = "bengine")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

#############
# Setup glm #
#############

http_archive(
    name = "glm",
    build_file = "@bengine//:third_party/glm/BUILD",
    sha256 = "37e2a3d62ea3322e43593c34bae29f57e3e251ea89f4067506c94043769ade4c",
    strip_prefix = "glm",
    urls = ["https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip"],
)

###########################
# Setup simplefilewatcher #
###########################

http_archive(
    name = "simple_file_watcher",
    build_file = "@bengine//:third_party/simple_file_watcher/BUILD",
    sha256 = "abe47cd43a65cdbbf960b2b162f31f0db7379f81418f4a9b4b5c9f3fdebf5a42",
    strip_prefix = "simplefilewatcher-ee0b97efd206282ef8bb4b9c10c90c941de4a52b",
    urls = ["https://github.com/apetrone/simplefilewatcher/archive/ee0b97efd206282ef8bb4b9c10c90c941de4a52b.zip"],
)

################
# Setup spdlog #
################

http_archive(
    name = "spdlog",
    build_file = "@bengine//:third_party/spdlog/BUILD",
    sha256 = "61f751265cfce8fb17f67e18fa1ad00077280c080008252d9e8f0fbab5c30662",
    strip_prefix = "spdlog-1.9.0",
    urls = ["https://github.com/gabime/spdlog/archive/v1.9.0.zip"],
)

###############
# Setup hayai #
###############

http_archive(
    name = "hayai",
    build_file = "@bengine//:third_party/hayai/BUILD",
    sha256 = "0bbda3c22b43c9fee1a6eb95662c25fe11afdcb3632c34e47717e198111cb6ce",
    strip_prefix = "hayai-1.0.2",
    urls = ["https://github.com/nickbruun/hayai/archive/v1.0.2.zip"],
)

################
# Setup vulkan #
################

http_archive(
    name = "com_github_zaucy_rules_7zip",
    sha256 = "b66e1c712577b0c029d4c94228dba9c8aacdcdeb88c3b1eeeffd00247ba5a856",
    strip_prefix = "rules_7zip-e95ba876db445cf2c925c02c4bc18ed37a503fd8",
    url = "https://github.com/zaucy/rules_7zip/archive/e95ba876db445cf2c925c02c4bc18ed37a503fd8.zip",
)

load("@com_github_zaucy_rules_7zip//:setup.bzl", "setup_7zip")

setup_7zip()

http_archive(
    name = "com_github_zaucy_rules_vulkan",
    sha256 = "7732cf33196c10ad581a935344d1822ae56adde7a41954cb777932e9fd619453",
    strip_prefix = "rules_vulkan-33e9a8d0c236dcfb21dc890ea319c36d7cfab010",
    url = "https://github.com/zaucy/rules_vulkan/archive/33e9a8d0c236dcfb21dc890ea319c36d7cfab010.zip",
)

load("@com_github_zaucy_rules_vulkan//:repo.bzl", "vulkan_repos")

vulkan_repos()

###############
# Setup imgui #
###############

http_archive(
    name = "imgui",
    build_file = "@bengine//:third_party/imgui/BUILD",
    sha256 = "3942f50a186d79c7673f4889b644fdb7ca6262e7c2f06094cf8be35218180b61",
    strip_prefix = "imgui-1.77",
    url = "https://github.com/ocornut/imgui/archive/v1.77.zip",
)

##############
# Setup glfw #
##############

http_archive(
    name = "glfw",
    build_file = "@bengine//:third_party/glfw/BUILD",
    sha256 = "33c6bfc422ca7002befbb39a7a60dc101d32c930ee58be532ffbcd52e9635812",
    strip_prefix = "glfw-3.3.2",
    url = "https://github.com/glfw/glfw/archive/3.3.2.zip",
)

################
# Setup abseil #
################

http_archive(
    name = "com_google_absl",
    patch_args = ["-p1"],
    patches = [
        "@bengine//:third_party/absl/0001-use-invoke_result-instead-of-result_of.patch",
    ],
    sha256 = "aabf6c57e3834f8dc3873a927f37eaf69975d4b28117fc7427dfb1c661542a87",
    strip_prefix = "abseil-cpp-98eb410c93ad059f9bba1bf43f5bb916fc92a5ea",
    urls = ["https://github.com/abseil/abseil-cpp/archive/98eb410c93ad059f9bba1bf43f5bb916fc92a5ea.zip"],
)

#############
# Setup stb #
#############

http_archive(
    name = "stb",
    build_file = "@bengine//:third_party/stb/BUILD",
    sha256 = "9dd7b5ff7538ecc8c65b9f392a0ed48f91c80a19b6f3f5a24cf1e687893fbe6b",
    strip_prefix = "stb-b42009b3b9d4ca35bc703f5310eedc74f584be58",
    url = "https://github.com/nothings/stb/archive/b42009b3b9d4ca35bc703f5310eedc74f584be58.zip",
)

################
# Setup catch2 #
################

http_archive(
    name = "catch2",
    patch_args = ["-p1"],
    patches = [
        "@bengine//:third_party/catch2/0001-allow-sharded-execution-for-bazel-test-sharding-supp.patch",
        "@bengine//:third_party/catch2/0002-patch-for-bengine.patch",
    ],
    sha256 = "8aec86bc5ccfd963caf3dc4cd137001a7f80bf54d44c69b187cc92b40851399a",
    strip_prefix = "Catch2-6f21a3609cea360846a0ca93be55877cca14c86d",
    url = "https://github.com/catchorg/Catch2/archive/6f21a3609cea360846a0ca93be55877cca14c86d.zip",
)

###############
# Setup boost #
###############

http_archive(
    name = "com_github_nelhage_rules_boost",
    patch_args = ["-p1"],
    patches = [
        "@bengine//:third_party/rules_boost/0001-add-patch_cmds_win.patch",
    ],
    sha256 = "9a588a62062c8bf352b398a5db3ccc65561d43c40968762f7e05da76ccb5a6c9",
    strip_prefix = "rules_boost-2613d04ab3d22dfc4543ea0a083d9adeaa0daf09",
    url = "https://github.com/nelhage/rules_boost/archive/2613d04ab3d22dfc4543ea0a083d9adeaa0daf09.zip",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

#############
# Setup fmt #
#############

http_archive(
    name = "fmt",
    build_file = "@bengine//:third_party/fmt/BUILD",
    sha256 = "cf962e2a76fc151cf22e108c176a9f0d1390bd70355178ee3639a762b898ce4e",
    strip_prefix = "fmt-7.0.3",
    url = "https://github.com/fmtlib/fmt/archive/7.0.3.zip",
)

#################
# Setup glslang #
#################

http_archive(
    name = "glslang",
    sha256 = "5ba7c7d8cbe33a8c6e9bd2a6127a80b22506e968b45f5c2da1cd2fc54d91b27b",
    strip_prefix = "glslang-11.5.0",
    url = "https://github.com/KhronosGroup/glslang/archive/refs/tags/11.5.0.zip",
)

#################
# Setup CLI11 #
#################

http_archive(
    name = "cli11",
    build_file = "@bengine//:third_party/cli11/BUILD",
    sha256 = "e488c75ca5077c302dfea153c4b67bda7fff53c470a8cedf9d7efbea169cca4a",
    strip_prefix = "CLI11-2.0.0",
    url = "https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.0.0.zip",
)

#################
# Setup CLI11 #
#################

http_archive(
    name = "nlohmann-json",
    build_file = "@bengine//:third_party/nlohmann-json/BUILD",
    sha256 = "b8ba5f82a64f0526c218fb8e5013eee002a0d39e25e18bfd0f59dfd3d3a01d3e",
    strip_prefix = "json-3.10.1",
    url = "https://github.com/nlohmann/json/archive/refs/tags/v3.10.1.zip",
)
