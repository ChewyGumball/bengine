// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Core/Logging/Logger.h>

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


#include <Renderer/Backends/Vulkan/VulkanCommandPool.h>
#include <Renderer/Backends/Vulkan/VulkanFence.h>
#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPhysicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineLayout.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineShaderStage.h>
#include <Renderer/Backends/Vulkan/VulkanQueue.h>
#include <Renderer/Backends/Vulkan/VulkanSemaphore.h>
#include <Renderer/Backends/Vulkan/VulkanShaderModule.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChain.h>

Core::LogCategory Test("Test");
// Core::LogCategory Vulkan("Vulkan");

Renderer::Backends::Vulkan::VulkanInstance instance;
Renderer::Backends::Vulkan::VulkanPhysicalDevice physicalDevice;
Renderer::Backends::Vulkan::VulkanLogicalDevice device;
Renderer::Backends::Vulkan::VulkanQueues queues;
VkSurfaceKHR surface;
Renderer::Backends::Vulkan::VulkanSwapChain swapChain;
VkRenderPass renderPass;
Renderer::Backends::Vulkan::VulkanPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
Renderer::Backends::Vulkan::VulkanCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;
Renderer::Backends::Vulkan::VulkanSemaphore imageAvailableSemaphore;
Renderer::Backends::Vulkan::VulkanSemaphore renderFinishedSemaphore;
Renderer::Backends::Vulkan::VulkanFence queueFence;

const std::vector<std::string> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<std::string> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

std::vector<std::string> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<std::string> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void cleanupVulkan() {
    using namespace Renderer::Backends::Vulkan;

    vkDeviceWaitIdle(device);

    VulkanSemaphore::Destroy(device, renderFinishedSemaphore);
    VulkanSemaphore::Destroy(device, imageAvailableSemaphore);
    VulkanFence::Destroy(device, queueFence);
    VulkanCommandPool::Destroy(device, commandPool);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    VulkanPipelineLayout::Destroy(device, pipelineLayout);
    vkDestroyRenderPass(device, renderPass, nullptr);

    VulkanSwapChain::Destroy(device, swapChain);
    VulkanLogicalDevice::Destroy(device);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    VulkanInstance::Destroy(instance);
}

VkResult createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = physicalDevice.swapChainDetails.format.format;
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

    pipelineLayout = VulkanPipelineLayout::Create(device);

    VulkanShaderModule vertexShader   = VulkanShaderModule::Create(device, "Content/Shaders/triangle/vert.spv");
    VulkanShaderModule fragmentShader = VulkanShaderModule::Create(device, "Content/Shaders/triangle/frag.spv");

    VulkanPipelineShaderStage vertexStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "main");
    VulkanPipelineShaderStage fragmentStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader, "main");

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexStage, fragmentStage};

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

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if(result != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create pipeline.");
    }

    VulkanShaderModule::Destroy(device, vertexShader);
    VulkanShaderModule::Destroy(device, fragmentShader);

    return result;
}

VkResult createCommandBuffers() {
    using namespace Renderer::Backends::Vulkan;
    commandBuffers.resize(swapChain.framebuffers.size());

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
        renderPassInfo.framebuffer           = swapChain.framebuffers[i];
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

bool initVulkan(GLFWwindow* window) {
    using namespace Renderer::Backends::Vulkan;
    std::vector<std::string> requiredExtensions = getRequiredExtensions();

    instance = Renderer::Backends::Vulkan::VulkanInstance::Create("Test", requiredExtensions, validationLayers);

    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create window surface!");
        return false;
    }

    auto maybePhysicalDevice = VulkanPhysicalDevice::Find(instance, surface, deviceExtensions);
    if(!maybePhysicalDevice) {
        return false;
    }

    physicalDevice = *maybePhysicalDevice;
    device = VulkanLogicalDevice::Create(physicalDevice.queueIndices, deviceExtensions, validationLayers);
    queues = VulkanQueues(device, physicalDevice.queueIndices);

    if(createRenderPass() != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create render pass.");
        return false;
    }

    swapChain = VulkanSwapChain::Create(device, physicalDevice.swapChainDetails, queues, renderPass);

    if(createGraphicsPipeline() != VK_SUCCESS) {
        return false;
    }

    commandPool = VulkanCommandPool::Create(device, queues.graphics.familyIndex);

    if(createCommandBuffers() != VK_SUCCESS) {
        Core::Log::Always(Vulkan, "Failed to create command buffers.");
        return false;
    }

    imageAvailableSemaphore = VulkanSemaphore::Create(device);
    renderFinishedSemaphore = VulkanSemaphore::Create(device);
    queueFence = VulkanFence::Create(device, VulkanFenceState::Signaled);

    return true;
}

VkResult drawFrame() {
    using namespace Renderer::Backends::Vulkan;
    queueFence.waitAndReset(device);

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