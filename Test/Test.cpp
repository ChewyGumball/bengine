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

#include <Renderer/Backends/Vulkan/VulkanCommandPool.h>
#include <Renderer/Backends/Vulkan/VulkanFence.h>
#include <Renderer/Backends/Vulkan/VulkanGraphicsPipeline.h>
#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPhysicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineLayout.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineShaderStage.h>
#include <Renderer/Backends/Vulkan/VulkanQueue.h>
#include <Renderer/Backends/Vulkan/VulkanRenderPass.h>
#include <Renderer/Backends/Vulkan/VulkanSemaphore.h>
#include <Renderer/Backends/Vulkan/VulkanShaderModule.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChain.h>

Core::LogCategory Test("Test");

Renderer::Backends::Vulkan::VulkanInstance instance;
Renderer::Backends::Vulkan::VulkanPhysicalDevice physicalDevice;
Renderer::Backends::Vulkan::VulkanLogicalDevice device;
Renderer::Backends::Vulkan::VulkanQueues queues;
VkSurfaceKHR surface;
Renderer::Backends::Vulkan::VulkanSwapChain swapChain;
Renderer::Backends::Vulkan::VulkanRenderPass renderPass;
Renderer::Backends::Vulkan::VulkanPipelineLayout pipelineLayout;
Renderer::Backends::Vulkan::VulkanGraphicsPipeline graphicsPipeline;
Renderer::Backends::Vulkan::VulkanCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;
Renderer::Backends::Vulkan::VulkanSemaphore imageAvailableSemaphore;
Renderer::Backends::Vulkan::VulkanSemaphore renderFinishedSemaphore;
Renderer::Backends::Vulkan::VulkanFence queueFence;

const std::vector<std::string> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const std::vector<std::string> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

std::vector<std::string> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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
    VulkanGraphicsPipeline::Destroy(device, graphicsPipeline);
    VulkanPipelineLayout::Destroy(device, pipelineLayout);
    VulkanRenderPass::Destroy(device, renderPass);
    VulkanSwapChain::Destroy(device, swapChain);
    VulkanLogicalDevice::Destroy(device);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    VulkanInstance::Destroy(instance);
}

void createGraphicsPipeline() {
    using namespace Renderer::Backends::Vulkan;

    VulkanShaderModule vertexShader   = VulkanShaderModule::Create(device, "Content/Shaders/triangle/vert.spv");
    VulkanShaderModule fragmentShader = VulkanShaderModule::Create(device, "Content/Shaders/triangle/frag.spv");

    VulkanPipelineShaderStage vertexStage(VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "main");
    VulkanPipelineShaderStage fragmentStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader, "main");

    VulkanGraphicsPipelineInfo info(swapChain.extent, pipelineLayout, renderPass);

    info.shaderStages.push_back(vertexStage);
    info.shaderStages.push_back(fragmentStage);

    graphicsPipeline = VulkanGraphicsPipeline::Create(device, info);

    VulkanShaderModule::Destroy(device, vertexShader);
    VulkanShaderModule::Destroy(device, fragmentShader);
}

void createCommandBuffers() {
    using namespace Renderer::Backends::Vulkan;
    commandBuffers = commandPool.AllocateBuffers(device, static_cast<uint32_t>(swapChain.framebuffers.size()));

    for(size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo         = nullptr;    // Optional

        VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass            = renderPass;
        renderPassInfo.framebuffer           = swapChain.framebuffers[i];
        renderPassInfo.renderArea.offset     = {0, 0};
        renderPassInfo.renderArea.extent     = swapChain.extent;

        VkClearValue clearColor        = {0.15f, 0.15f, 0.15f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues    = &clearColor;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);

        VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
    }
}

bool initVulkan(GLFWwindow* window) {
    using namespace Renderer::Backends::Vulkan;
    instance = Renderer::Backends::Vulkan::VulkanInstance::Create("Test", getRequiredExtensions(), validationLayers);

    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));

    auto maybePhysicalDevice = VulkanPhysicalDevice::Find(instance, surface, deviceExtensions);
    if(!maybePhysicalDevice) {
        return false;
    }

    physicalDevice = *maybePhysicalDevice;
    device         = VulkanLogicalDevice::Create(physicalDevice.queueIndices, deviceExtensions, validationLayers);
    queues         = VulkanQueues(device, physicalDevice.queueIndices);
    renderPass     = VulkanRenderPass::Create(device, physicalDevice.swapChainDetails.format.format);
    swapChain      = VulkanSwapChain::Create(device, physicalDevice.swapChainDetails, queues, renderPass);
    pipelineLayout = VulkanPipelineLayout::Create(device);
    createGraphicsPipeline();

    commandPool = VulkanCommandPool::Create(device, queues.graphics.familyIndex);
    createCommandBuffers();

    imageAvailableSemaphore = VulkanSemaphore::Create(device);
    renderFinishedSemaphore = VulkanSemaphore::Create(device);
    queueFence              = VulkanFence::Create(device, VulkanFenceState::Signaled);

    return true;
}

void drawFrame() {
    using namespace Renderer::Backends::Vulkan;
    queueFence.waitAndReset(device);

    uint32_t imageIndex = swapChain.aquireNextImage(device, imageAvailableSemaphore);
    queues.graphics.submit(commandBuffers[imageIndex], imageAvailableSemaphore, renderFinishedSemaphore, queueFence);
    queues.present.present(swapChain, renderFinishedSemaphore, imageIndex);
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