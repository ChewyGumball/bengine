#include "Renderer/Backends/Vulkan/VulkanImage.h"

namespace internal {
VkImageUsageFlags TranslateImageType(Renderer::Backends::Vulkan::VulkanImageUsageType usageType,
                                     Renderer::Backends::Vulkan::VulkanImageTransferType transferType) {
    using namespace Renderer::Backends::Vulkan;

    VkImageUsageFlags flags = 0;

    if(transferType == VulkanImageTransferType::Source) {
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if(transferType == VulkanImageTransferType::Destination) {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if(usageType == VulkanImageUsageType::Sampled) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if(usageType == VulkanImageUsageType::Storage) {
        flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if(usageType == VulkanImageUsageType::Colour) {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if(usageType == VulkanImageUsageType::Depth) {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if(usageType == VulkanImageUsageType::Input) {
        flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    return flags;
}
}    // namespace internal

namespace Renderer::Backends::Vulkan {
void VulkanImage::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier            = {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = object;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(VulkanImage::hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
              newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        // VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else {
        Core::Log::Error(Vulkan, "Unknown layout types");
        VK_CHECK(VK_ERROR_FORMAT_NOT_SUPPORTED);
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

bool VulkanImage::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


VulkanImage VulkanImage::Create(VmaAllocator allocator,
                                VkExtent2D dimensions,
                                VkFormat format,
                                VulkanImageUsageType usageType,
                                VulkanImageTransferType transferType,
                                VulkanMemoryVisibility visibility) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType         = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width      = dimensions.width;
    imageInfo.extent.height     = dimensions.height;
    imageInfo.extent.depth      = 1;
    imageInfo.mipLevels         = 1;
    imageInfo.arrayLayers       = 1;
    imageInfo.format            = format;
    imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage             = internal::TranslateImageType(usageType, transferType);
    imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags             = 0;    // Optional

    VulkanImage image;
    image.allocator     = allocator;
    image.extent.width  = dimensions.width;
    image.extent.height = dimensions.height;
    image.extent.depth  = 1;
    image.format        = format;
    image.size          = dimensions.width * dimensions.height * sizeof(uint32_t);
    image.usageType     = usageType;
    image.transferType  = transferType;
    image.visibility    = visibility;


    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage                   = TranslateMemoryType(visibility);

    VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image.object, &image.allocation, nullptr));

    return image;
}

void VulkanImage::Destroy(VulkanImage& image) {
    vmaDestroyImage(image.allocator, image.object, image.allocation);
}


}    // namespace Renderer::Backends::Vulkan