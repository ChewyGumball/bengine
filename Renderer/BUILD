load("//tools:bengine_rules.bzl", "bengine_cc_library")

package(default_visibility = ["//visibility:public"])

bengine_cc_library(
    name = "vulkan",
    srcs = [
        "src/Backends/Vulkan/DiagnosticCheckpoint.cpp",
        "src/Backends/Vulkan/VulkanBuffer.cpp",
        "src/Backends/Vulkan/VulkanCommandPool.cpp",
        "src/Backends/Vulkan/VulkanCore.cpp",
        "src/Backends/Vulkan/VulkanDescriptorPool.cpp",
        "src/Backends/Vulkan/VulkanFence.cpp",
        "src/Backends/Vulkan/VulkanFramebuffer.cpp",
        "src/Backends/Vulkan/VulkanGraphicsPipeline.cpp",
        "src/Backends/Vulkan/VulkanImage.cpp",
        "src/Backends/Vulkan/VulkanImageView.cpp",
        "src/Backends/Vulkan/VulkanInstance.cpp",
        "src/Backends/Vulkan/VulkanLogicalDevice.cpp",
        "src/Backends/Vulkan/VulkanPhysicalDevice.cpp",
        "src/Backends/Vulkan/VulkanPipelineLayout.cpp",
        "src/Backends/Vulkan/VulkanQueue.cpp",
        "src/Backends/Vulkan/VulkanRenderPass.cpp",
        "src/Backends/Vulkan/VulkanSampler.cpp",
        "src/Backends/Vulkan/VulkanSemaphore.cpp",
        "src/Backends/Vulkan/VulkanShaderModule.cpp",
        "src/Backends/Vulkan/VulkanSwapChain.cpp",
        "src/Backends/Vulkan/VulkanSwapChainDetails.cpp",
    ],
    hdrs = [
        "include/Renderer/Backends/Vulkan/DiagnosticCheckpoint.h",
        "include/Renderer/Backends/Vulkan/VulkanBuffer.h",
        "include/Renderer/Backends/Vulkan/VulkanCommandPool.h",
        "include/Renderer/Backends/Vulkan/VulkanCore.h",
        "include/Renderer/Backends/Vulkan/VulkanDescriptorPool.h",
        "include/Renderer/Backends/Vulkan/VulkanFence.h",
        "include/Renderer/Backends/Vulkan/VulkanFramebuffer.h",
        "include/Renderer/Backends/Vulkan/VulkanGraphicsPipeline.h",
        "include/Renderer/Backends/Vulkan/VulkanImage.h",
        "include/Renderer/Backends/Vulkan/VulkanImageView.h",
        "include/Renderer/Backends/Vulkan/VulkanInstance.h",
        "include/Renderer/Backends/Vulkan/VulkanLogicalDevice.h",
        "include/Renderer/Backends/Vulkan/VulkanPhysicalDevice.h",
        "include/Renderer/Backends/Vulkan/VulkanPipelineLayout.h",
        "include/Renderer/Backends/Vulkan/VulkanPipelineShaderStage.h",
        "include/Renderer/Backends/Vulkan/VulkanQueue.h",
        "include/Renderer/Backends/Vulkan/VulkanRenderPass.h",
        "include/Renderer/Backends/Vulkan/VulkanSampler.h",
        "include/Renderer/Backends/Vulkan/VulkanSemaphore.h",
        "include/Renderer/Backends/Vulkan/VulkanShaderModule.h",
        "include/Renderer/Backends/Vulkan/VulkanSwapChain.h",
        "include/Renderer/Backends/Vulkan/VulkanSwapChainDetails.h",
    ],
    strip_include_prefix = "include",
    deps = [
        "//core/algorithms:containers",
        "//core/algorithms:mappers",
        "//core/algorithms:optional",
        "//core/io/file_system",
        "//core/logging",
        "//core/status",
        "@vulkan_sdk_windows//:vulkan",
    ],
)

bengine_cc_library(
    name = "resources",
    hdrs = [
        "include/Renderer/Resources/GPUMesh.h",
        "include/Renderer/Resources/GPUTexture.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":vulkan",
    ],
)

bengine_cc_library(
    name = "vulkan_backend",
    srcs = ["src/Backends/Vulkan/VulkanRendererBackend.cpp"],
    hdrs = [
        "include/Renderer/Backends/RendererBackend.h",
        "include/Renderer/Backends/Vulkan/VulkanRendererBackend.h",
    ],
    strip_include_prefix = "include",
    deps = [
        ":resources",
        ":vulkan",
        "//core/containers:array",
        "//core/containers:hash_set",
        "//core/status",
    ],
)
