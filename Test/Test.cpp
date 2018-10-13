// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Core/Logging/Logger.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <array>
#include <optional>

#include <Renderer/Backends/Vulkan/VulkanBuffer.h>
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
Renderer::Backends::Vulkan::VulkanCommandPool transientTransferCommandPool;
Renderer::Backends::Vulkan::VulkanCommandPool drawingCommandPool;
std::vector<VkCommandBuffer> commandBuffers;
std::vector<Renderer::Backends::Vulkan::VulkanSemaphore> imageAvailableSemaphores;
std::vector<Renderer::Backends::Vulkan::VulkanSemaphore> renderFinishedSemaphores;
std::vector<Renderer::Backends::Vulkan::VulkanFence> queueFences;

Renderer::Backends::Vulkan::VulkanBuffer VertexBuffer;
Renderer::Backends::Vulkan::VulkanBuffer IndexBuffer;


const std::vector<std::string> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const std::vector<std::string> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
const int MAX_FRAME_IN_FLIGHT                   = 2;
size_t currentFrame                             = 0;
bool framebufferResized                         = false;

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding                         = 0;
        bindingDescription.stride                          = sizeof(Vertex);
        bindingDescription.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos);

        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
const std::vector<Vertex> vertices  = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};


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

    drawingCommandPool.freeBuffers(device, commandBuffers);
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

    physicalDevice.destroyBuffer(device, VertexBuffer);
    physicalDevice.destroyBuffer(device, IndexBuffer);

    VulkanPipelineLayout::Destroy(device, pipelineLayout);
    VulkanCommandPool::Destroy(device, transientTransferCommandPool);
    VulkanCommandPool::Destroy(device, drawingCommandPool);
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

    auto bindingDescription    = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    info.shaderStages.push_back(vertexStage);
    info.shaderStages.push_back(fragmentStage);
    info.vertexInput.vertexBindingDescriptionCount   = 1;
    info.vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    info.vertexInput.pVertexBindingDescriptions      = &bindingDescription;
    info.vertexInput.pVertexAttributeDescriptions    = attributeDescriptions.data();

    graphicsPipeline = VulkanGraphicsPipeline::Create(device, info);

    VulkanShaderModule::Destroy(device, vertexShader);
    VulkanShaderModule::Destroy(device, fragmentShader);
}

void createCommandBuffers() {
    using namespace Renderer::Backends::Vulkan;
    commandBuffers = drawingCommandPool.allocateBuffers(device, static_cast<uint32_t>(swapChain.framebuffers.size()));

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

        VkBuffer vertexBuffers[] = {VertexBuffer};
        VkDeviceSize offsets[]   = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[i], IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffers[i]);

        VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
    }
}

VkCommandBuffer copyBuffer(Renderer::Backends::Vulkan::VulkanBuffer& source,
                           Renderer::Backends::Vulkan::VulkanBuffer& destination) {
    using namespace Renderer::Backends::Vulkan;

    VkCommandBuffer copyBuffer = transientTransferCommandPool.allocateBuffers(device, 1)[0];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(copyBuffer, &beginInfo));

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset    = 0;    // Optional
    copyRegion.dstOffset    = 0;    // Optional
    copyRegion.size         = source.size;
    vkCmdCopyBuffer(copyBuffer, source, destination, 1, &copyRegion);
    vkEndCommandBuffer(copyBuffer);

    queues.transfer.submit(copyBuffer, VulkanQueueSubmitType::Transfer);
    return copyBuffer;
}

void createVertexBuffer() {
    using namespace Renderer::Backends::Vulkan;

    uint32_t vertexSize              = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
    VulkanBuffer vertexStagingBuffer = physicalDevice.createBuffer(device,
                                                                   vertexSize,
                                                                   VulkanBufferUsageType::None,
                                                                   VulkanBufferTransferType::Source,
                                                                   VulkanBufferDeviceVisibility::Host);

    std::byte* mappedData = vertexStagingBuffer.Map(device);
    std::memcpy(mappedData, vertices.data(), vertexStagingBuffer.size);
    vertexStagingBuffer.Unmap(device);    // maybe not required

    VertexBuffer = physicalDevice.createBuffer(device,
                                               vertexSize,
                                               VulkanBufferUsageType::Vertex,
                                               VulkanBufferTransferType::Destination,
                                               VulkanBufferDeviceVisibility::Device);

    VkCommandBuffer vertexCopyBuffer = copyBuffer(vertexStagingBuffer, VertexBuffer);


    uint32_t indexSize              = static_cast<uint32_t>(indices.size() * sizeof(uint16_t));
    VulkanBuffer indexStagingBuffer = physicalDevice.createBuffer(device,
                                                                  vertexSize,
                                                                  VulkanBufferUsageType::None,
                                                                  VulkanBufferTransferType::Source,
                                                                  VulkanBufferDeviceVisibility::Host);

    mappedData = indexStagingBuffer.Map(device);
    std::memcpy(mappedData, indices.data(), indexStagingBuffer.size);
    indexStagingBuffer.Unmap(device);    // maybe not required

    IndexBuffer = physicalDevice.createBuffer(device,
                                              vertexSize,
                                              VulkanBufferUsageType::Index,
                                              VulkanBufferTransferType::Destination,
                                              VulkanBufferDeviceVisibility::Device);

    VkCommandBuffer indexCopyBuffer = copyBuffer(indexStagingBuffer, IndexBuffer);

    vkQueueWaitIdle(queues.transfer);
    transientTransferCommandPool.freeBuffers(device, {vertexCopyBuffer, indexCopyBuffer});
    physicalDevice.destroyBuffer(device, vertexStagingBuffer);
    physicalDevice.destroyBuffer(device, indexStagingBuffer);
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
    transientTransferCommandPool =
          VulkanCommandPool::Create(device, queues.transfer.familyIndex, VulkanCommandBufferLifetime::Transient);
    drawingCommandPool = VulkanCommandPool::Create(device, queues.graphics.familyIndex);

    createVertexBuffer();

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
                           VulkanQueueSubmitType::Graphics,
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