// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <optional>
#include <set>

#include <Core/Algorithms/Containers.h>
#include <Core/Algorithms/Mappers.h>
#include <Core/File.h>

#include <fstream>

#include <Core/Logging/Logger.h>

#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanQueue.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChain.h>

Core::LogCategory Test("Test");
// Core::LogCategory Vulkan("Vulkan");

Renderer::Backends::Vulkan::VulkanInstance instance;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
Renderer::Backends::Vulkan::VulkanLogicalDevice device;
Renderer::Backends::Vulkan::VulkanQueues queues;
VkSurfaceKHR surface;
Renderer::Backends::Vulkan::VulkanSwapChain swapChain;
std::vector<VkImage> swapChainImages;
std::vector<VkImageView> swapChainImageViews;
VkShaderModule vertShaderModule;
VkShaderModule fragShaderModule;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
std::vector<VkFramebuffer> swapChainFramebuffers;
VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;
VkSemaphore imageAvailableSemaphore;
VkSemaphore renderFinishedSemaphore;
VkFence queueFence;

const std::vector<std::string> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<std::string> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

std::vector<const char*> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

std::string vendorNameFromID(uint32_t vendorID) {
    std::unordered_map<uint32_t, std::string> vendorNames{{0x1002, "AMD"},
                                                          {0x1010, "ImgTec"},
                                                          {0x10DE, "NVIDIA"},
                                                          {0x13B5, "ARM"},
                                                          {0x5143, "Qualcomm"},
                                                          {0x8086, "INTEL"}};

    auto mappedName = vendorNames.find(vendorID);
    if(mappedName != vendorNames.end()) {
        return mappedName->second;
    } else {
        return "Unknown Vendor";
    }
}

std::string deviceTypeNameFromEnum(VkPhysicalDeviceType type) {
    switch(type) {
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        default: return "Unknown Device Type";
    }
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

QueueFamilyIndices findQueueFamilyIndicies(VkPhysicalDevice deviceToCheck) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(deviceToCheck, &queueFamilyCount, queueFamilies.data());

    QueueFamilyIndices indices;
    for(uint32_t i = 0; i < queueFamilies.size() && !indices.isComplete(); i++) {
        if(queueFamilies[i].queueCount > 0) {
            if((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(deviceToCheck, i, surface, &presentSupport);
            if(presentSupport) {
                indices.presentFamily = i;
            }
        }
    }

    return indices;
}
bool deviceSupportsRequiredExtensions(VkPhysicalDevice deviceToCheck) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(deviceToCheck, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for(const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice deviceToCheck) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceToCheck, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToCheck, surface, &formatCount, nullptr);

    if(formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(deviceToCheck, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(deviceToCheck, surface, &presentModeCount, nullptr);

    if(presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
              deviceToCheck, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkResult pickDevice() {
    using namespace Renderer::Backends::Vulkan;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if(deviceCount == 0) {
        Core::Log::Always(Vulkan, "No devices support Vulkan!");
        return VK_ERROR_DEVICE_LOST;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for(auto device : devices) {
        bool extensionsSupported = deviceSupportsRequiredExtensions(device);

        bool swapChainAdequate = false;
        if(extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        if(findQueueFamilyIndicies(device).isComplete() && swapChainAdequate) {
            physicalDevice = device;
            break;
        }
    }

    if(physicalDevice == VK_NULL_HANDLE) {
        Core::Log::Always(Vulkan, "No devices support all required queue types!");
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);


    Core::Log::Debug(Vulkan,
                     "Using {} [Vendor: {}, Type: {}, Driver Version: {}, API Version: {}]",
                     deviceProperties.deviceName,
                     vendorNameFromID(deviceProperties.vendorID),
                     deviceTypeNameFromEnum(deviceProperties.deviceType),
                     deviceProperties.driverVersion,
                     deviceProperties.apiVersion);

    return VK_SUCCESS;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    if(availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    auto predicate = [](auto f) {
        return f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    };
    return Core::Algorithms::FindIf(availableFormats, predicate).value_or(availableFormats[0]);
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {800, 600};

        actualExtent.width =
              std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height =
              std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkResult createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for(size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo           = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                           = swapChainImages[i];
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = swapChain.imageFormat;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        VkResult createResult = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);
        if(createResult != VK_SUCCESS) {
            return createResult;
        }
    }

    return VK_SUCCESS;
}

void cleanupVulkan() {
    using namespace Renderer::Backends::Vulkan;

    vkDeviceWaitIdle(device);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroyFence(device, queueFence, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    for(auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    for(auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    VulkanSwapChain::Destroy(device, swapChain);
    VulkanLogicalDevice::Destroy(device);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    VulkanInstance::Destroy(instance);
}

VkResult createShaderModule(const std::vector<std::byte>& code, VkShaderModule& shaderModule) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = code.size();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*>(code.data());

    return vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
}


VkResult createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = swapChain.imageFormat;
    colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment            = 0;
    colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask       = 0;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount        = 1;
    renderPassInfo.pAttachments           = &colorAttachment;
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;
    renderPassInfo.dependencyCount        = 1;
    renderPassInfo.pDependencies          = &dependency;

    return vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
}

VkResult createGraphicsPipeline() {
    using namespace Renderer::Backends::Vulkan;
    std::optional<std::vector<std::byte>> vertexShaderCode =
          Core::File::ReadBinaryFile("Content/Shaders/triangle/vert.spv");
    std::optional<std::vector<std::byte>> fragShaderCode =
          Core::File::ReadBinaryFile("Content/Shaders/triangle/frag.spv");


    if(!vertexShaderCode) {
        Core::Log::Always(Vulkan, "Failed to load vertex shader code.");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if(!fragShaderCode) {
        Core::Log::Always(Vulkan, "Failed to load fragment shader code.");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkResult result = createShaderModule(*vertexShaderCode, vertShaderModule);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create vertex shader module.");
        return result;
    }

    result = createShaderModule(*fragShaderCode, fragShaderModule);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create fragment shader module.");
        vkDestroyShaderModule(device, vertShaderModule, nullptr);

        return result;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage                           = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module                          = vertShaderModule;
    vertShaderStageInfo.pName                           = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module                          = fragShaderModule;
    fragShaderStageInfo.pName                           = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount        = 0;
    vertexInputInfo.pVertexBindingDescriptions           = nullptr;    // Optional
    vertexInputInfo.vertexAttributeDescriptionCount      = 0;
    vertexInputInfo.pVertexAttributeDescriptions         = nullptr;    // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable                 = VK_FALSE;

    VkViewport viewport = {};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = (float)swapChain.extent.width;
    viewport.height     = (float)swapChain.extent.height;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor = {};
    scissor.offset   = {0, 0};
    scissor.extent   = swapChain.extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount                     = 1;
    viewportState.pViewports                        = &viewport;
    viewportState.scissorCount                      = 1;
    viewportState.pScissors                         = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable                       = VK_FALSE;
    rasterizer.rasterizerDiscardEnable                = VK_FALSE;
    rasterizer.polygonMode                            = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth                              = 1.0f;
    rasterizer.cullMode                               = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace                              = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable                        = VK_FALSE;
    rasterizer.depthBiasConstantFactor                = 0.0f;    // Optional
    rasterizer.depthBiasClamp                         = 0.0f;    // Optional
    rasterizer.depthBiasSlopeFactor                   = 0.0f;    // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable                  = VK_FALSE;
    multisampling.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading                     = 1.0f;        // Optional
    multisampling.pSampleMask                          = nullptr;     // Optional
    multisampling.alphaToCoverageEnable                = VK_FALSE;    // Optional
    multisampling.alphaToOneEnable                     = VK_FALSE;    // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask =
          VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;     // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;    // Optional
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;         // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;     // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;    // Optional
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;         // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable                       = VK_FALSE;
    colorBlending.logicOp                             = VK_LOGIC_OP_COPY;    // Optional
    colorBlending.attachmentCount                     = 1;
    colorBlending.pAttachments                        = &colorBlendAttachment;
    colorBlending.blendConstants[0]                   = 0.0f;    // Optional
    colorBlending.blendConstants[1]                   = 0.0f;    // Optional
    colorBlending.blendConstants[2]                   = 0.0f;    // Optional
    colorBlending.blendConstants[3]                   = 0.0f;    // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount             = 0;          // Optional
    pipelineLayoutInfo.pSetLayouts                = nullptr;    // Optional
    pipelineLayoutInfo.pushConstantRangeCount     = 0;          // Optional
    pipelineLayoutInfo.pPushConstantRanges        = nullptr;    // Optional

    result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create pipeline layout.");

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);

        return result;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount                   = 2;
    pipelineInfo.pStages                      = shaderStages;
    pipelineInfo.pVertexInputState            = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState          = &inputAssembly;
    pipelineInfo.pViewportState               = &viewportState;
    pipelineInfo.pRasterizationState          = &rasterizer;
    pipelineInfo.pMultisampleState            = &multisampling;
    pipelineInfo.pDepthStencilState           = nullptr;    // Optional
    pipelineInfo.pColorBlendState             = &colorBlending;
    pipelineInfo.pDynamicState                = nullptr;    // Optional
    pipelineInfo.layout                       = pipelineLayout;
    pipelineInfo.renderPass                   = renderPass;
    pipelineInfo.subpass                      = 0;
    pipelineInfo.basePipelineHandle           = VK_NULL_HANDLE;    // Optional
    pipelineInfo.basePipelineIndex            = -1;                // Optional

    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create pipeline.");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    return result;
}

VkResult createFramebuffers() {
    using namespace Renderer::Backends::Vulkan;
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for(size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass              = renderPass;
        framebufferInfo.attachmentCount         = 1;
        framebufferInfo.pAttachments            = attachments;
        framebufferInfo.width                   = swapChain.extent.width;
        framebufferInfo.height                  = swapChain.extent.height;
        framebufferInfo.layers                  = 1;

        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);
        if(result != VK_SUCCESS) {
            Core::Log::Always(Vulkan, "Failed to create framebuffer {}!", i);
            return result;
        }
    }

    return VK_SUCCESS;
}

VkResult createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilyIndicies(physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex        = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags                   = 0;    // Optional

    return vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
}

VkResult createCommandBuffers() {
    using namespace Renderer::Backends::Vulkan;
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                 = commandPool;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount          = (uint32_t)commandBuffers.size();

    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to allocate command buffers.");
        return result;
    }

    for(size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo         = nullptr;    // Optional

        result = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
        if(result != VK_SUCCESS) {
            Core::Log::Always(Vulkan, "Failed to begin command buffer.");
            return result;
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass            = renderPass;
        renderPassInfo.framebuffer           = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset     = {0, 0};
        renderPassInfo.renderArea.extent     = swapChain.extent;

        VkClearValue clearColor        = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues    = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);

        result = vkEndCommandBuffer(commandBuffers[i]);
        if(result != VK_SUCCESS) {
            Core::Log::Always(Vulkan, "Failed to end command buffer.");
            return result;
        }
    }

    return VK_SUCCESS;
}

VkResult createSemaphores() {
    using namespace Renderer::Backends::Vulkan;
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create image available semaphore.");
        return result;
    }

    result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create render finished semaphore.");
        return result;
    }

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    result = vkCreateFence(device, &fenceInfo, nullptr, &queueFence);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create fence.");
    }

    return result;
}

bool initVulkan(GLFWwindow* window) {
    using namespace Renderer::Backends::Vulkan;
    auto extensions = getRequiredExtensions();
    std::vector<std::string> requiredExtensions;
    Core::Algorithms::Map(extensions, requiredExtensions, Core::Algorithms::Mappers::CharToString());

    instance = Renderer::Backends::Vulkan::VulkanInstance::Create("Test", requiredExtensions, validationLayers);

    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create window surface!");
        return false;
    }

    if(pickDevice() != VK_SUCCESS) {
        return false;
    }

    VulkanQueueFamilyIndices queueFamilyIndices = *VulkanQueueFamilyIndices::Find(physicalDevice, surface);

    device = VulkanLogicalDevice::Create(queueFamilyIndices, deviceExtensions, validationLayers);
    queues = VulkanQueues(device, queueFamilyIndices);
    swapChain = Renderer::Backends::Vulkan::VulkanSwapChain::Create(physicalDevice, device, surface, queues);

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    if(createImageViews() != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create swap chain image views!");
        return false;
    }

    if(createRenderPass() != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create render pass.");
        return false;
    }

    if(createGraphicsPipeline() != VK_SUCCESS) {
        return false;
    }

    if(createFramebuffers() != VK_SUCCESS) {
        return false;
    }

    if(createCommandPool() != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create command pool.");
        return false;
    }

    if(createCommandBuffers() != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create command buffers.");
        return false;
    }

    if(createSemaphores() != VK_SUCCESS) {
        return false;
    }


    return true;
}
VkResult drawFrame() {
    using namespace Renderer::Backends::Vulkan;
    vkWaitForFences(device, 1, &queueFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &queueFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device,
                          swapChain,
                          std::numeric_limits<uint64_t>::max(),
                          imageAvailableSemaphore,
                          VK_NULL_HANDLE,
                          &imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[]      = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount     = 1;
    submitInfo.pWaitSemaphores        = waitSemaphores;
    submitInfo.pWaitDstStageMask      = waitStages;
    submitInfo.commandBufferCount     = 1;
    submitInfo.pCommandBuffers        = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[]  = {renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    VkResult result = vkQueueSubmit(queues.graphics, 1, &submitInfo, queueFence);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to submit queue.");
        return result;
    }

    VkSwapchainKHR swapChains[] = {swapChain};

    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapChains;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;    // Optional

    return vkQueuePresentKHR(queues.present, &presentInfo);
}

int main() {
    using namespace Renderer::Backends::Vulkan;
    Core::LogManager::SetCategoryLevel(Vulkan, Core::LogLevel::Trace);

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    if(initVulkan(window)) {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
    }

    cleanupVulkan();

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}