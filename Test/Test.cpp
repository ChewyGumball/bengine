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
std::vector<Renderer::Backends::Vulkan::VulkanSemaphore> imageAvailableSemaphores;
std::vector<Renderer::Backends::Vulkan::VulkanSemaphore> renderFinishedSemaphores;
std::vector<Renderer::Backends::Vulkan::VulkanFence> queueFences;

const std::vector<std::string> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const std::vector<std::string> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
const int MAX_FRAME_IN_FLIGHT                   = 2;
size_t currentFrame                             = 0;
bool framebufferResized                         = false;

VkExtent2D GetWindowSize(GLFWwindow* window) {
    int32_t width, height;
    glfwGetFramebufferSize(window, &width, &height);

    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

std::vector<std::string> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<std::string> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void cleanupSwapChain() {
    using namespace Renderer::Backends::Vulkan;

    commandPool.freeBuffers(device, commandBuffers);
    VulkanGraphicsPipeline::Destroy(device, graphicsPipeline);
    VulkanRenderPass::Destroy(device, renderPass);
    VulkanSwapChain::Destroy(device, swapChain);
}

void cleanupVulkan() {
    using namespace Renderer::Backends::Vulkan;

    vkDeviceWaitIdle(device);

    for(int i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        VulkanSemaphore::Destroy(device, renderFinishedSemaphores[i]);
        VulkanSemaphore::Destroy(device, imageAvailableSemaphores[i]);
        VulkanFence::Destroy(device, queueFences[i]);
    }

    cleanupSwapChain();

    VulkanPipelineLayout::Destroy(device, pipelineLayout);
    VulkanCommandPool::Destroy(device, commandPool);
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
    commandBuffers = commandPool.allocateBuffers(device, static_cast<uint32_t>(swapChain.framebuffers.size()));

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

    auto maybePhysicalDevice = VulkanPhysicalDevice::Find(instance, surface, deviceExtensions, GetWindowSize(window));
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

    imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    queueFences.resize(MAX_FRAME_IN_FLIGHT);
    for(int i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = VulkanSemaphore::Create(device);
        renderFinishedSemaphores[i] = VulkanSemaphore::Create(device);
        queueFences[i]              = VulkanFence::Create(device, VulkanFenceState::Signaled);
    }


    return true;
}

void recreateSwapChain(GLFWwindow* window) {
    using namespace Renderer::Backends::Vulkan;

    int width = 0, height = 0;
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);
    cleanupSwapChain();

    physicalDevice.swapChainDetails = VulkanSwapChainDetails::Find(physicalDevice, surface, GetWindowSize(window));

    renderPass = VulkanRenderPass::Create(device, physicalDevice.swapChainDetails.format.format);
    swapChain  = VulkanSwapChain::Create(device, physicalDevice.swapChainDetails, queues, renderPass);
    createGraphicsPipeline();
    createCommandBuffers();
}

void drawFrame(GLFWwindow* window) {
    using namespace Renderer::Backends::Vulkan;

    auto acquisitionResult = swapChain.acquireNextImage(device, imageAvailableSemaphores[currentFrame]);

    if(acquisitionResult.result == VK_ERROR_OUT_OF_DATE_KHR || acquisitionResult.result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain(window);
        return;
    } else if(acquisitionResult.result != VK_SUCCESS) {
        Core::Log::Error(Test, "Couldn't acquire swap chain image: {}", acquisitionResult.result);
        return;
    }

    queueFences[currentFrame].waitAndReset(device);
    queues.graphics.submit(commandBuffers[acquisitionResult.imageIndex],
                           imageAvailableSemaphores[currentFrame],
                           renderFinishedSemaphores[currentFrame],
                           queueFences[currentFrame]);
    VkResult presentResult =
          queues.present.present(swapChain, renderFinishedSemaphores[currentFrame], acquisitionResult.imageIndex);

    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain(window);
    } else if(presentResult != VK_SUCCESS) {
        Core::Log::Error(Test, "Failed to present image: {}", presentResult);
    }

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
}


void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    framebufferResized = true;
}

GLFWwindow* initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    return window;
}

int main() {
    using namespace Renderer::Backends::Vulkan;
    Core::LogManager::SetCategoryLevel(Vulkan, Core::LogLevel::Trace);

    GLFWwindow* window = initWindow();

    if(initVulkan(window)) {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame(window);
        }
    }

    cleanupVulkan();

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}