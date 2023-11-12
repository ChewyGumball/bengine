#pragma once

#include <Renderer/Backends/Vulkan/VulkanImage.h>
#include <Renderer/Backends/Vulkan/VulkanImageView.h>
#include <Renderer/Backends/Vulkan/VulkanSampler.h>

namespace Renderer::Resources {

struct GPUTexture {
    Renderer::Backends::Vulkan::VulkanImage image;
    Renderer::Backends::Vulkan::VulkanImageView view;
    Renderer::Backends::Vulkan::VulkanSampler sampler;
};

}    // namespace Renderer::Resources
