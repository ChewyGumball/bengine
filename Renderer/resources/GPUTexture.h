#pragma once

#include "renderer/backends/vulkan/VulkanImage.h"
#include "renderer/backends/vulkan/VulkanImageView.h"
#include "renderer/backends/vulkan/VulkanSampler.h"

namespace Renderer::Resources {

struct GPUTexture {
    Renderer::Backends::Vulkan::VulkanImage image;
    Renderer::Backends::Vulkan::VulkanImageView view;
    Renderer::Backends::Vulkan::VulkanSampler sampler;
};

}    // namespace Renderer::Resources
