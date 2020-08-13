// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Core/Logging/Logger.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <chrono>
#include <optional>

#include <Core/IO/FileSystem/FileSystem.h>
#include <Core/IO/FileSystem/Path.h>
#include <Core/IO/FileSystem/VirtualFileSystemMount.h>
#include <Core/Time/SystemClockTicker.h>
#include <Core/Time/Timer.h>

#include <Core/IO/BufferView.h>

#include <Renderer/Backends/Vulkan/VulkanBuffer.h>
#include <Renderer/Backends/Vulkan/VulkanCommandPool.h>
#include <Renderer/Backends/Vulkan/VulkanDescriptorPool.h>
#include <Renderer/Backends/Vulkan/VulkanFence.h>
#include <Renderer/Backends/Vulkan/VulkanGraphicsPipeline.h>
#include <Renderer/Backends/Vulkan/VulkanImage.h>
#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPhysicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineLayout.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineShaderStage.h>
#include <Renderer/Backends/Vulkan/VulkanQueue.h>
#include <Renderer/Backends/Vulkan/VulkanRenderPass.h>
#include <Renderer/Backends/Vulkan/VulkanSampler.h>
#include <Renderer/Backends/Vulkan/VulkanSemaphore.h>
#include <Renderer/Backends/Vulkan/VulkanShaderModule.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChain.h>

#include <absl/container/flat_hash_map.h>

#include <Assets/Importers/OBJImporter.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <imgui.h>

#include <sstream>


#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

Core::LogCategory Test("Test");
using namespace Renderer::Backends::Vulkan;

VulkanInstance instance;
VulkanPhysicalDevice physicalDevice;
VulkanLogicalDevice device;
VulkanQueues queues;
VkSurfaceKHR surface;
VkDescriptorSetLayout descriptorSetLayout;
VulkanDescriptorPool descriptorPool;
VulkanSwapChain swapChain;
VulkanRenderPass renderPass;
VulkanPipelineLayout pipelineLayout;
VulkanGraphicsPipeline graphicsPipeline;
std::vector<VkDescriptorSet> uniformBuffersDescriptors;
std::vector<VulkanBuffer> uniformBuffers;
std::vector<VkCommandBuffer> commandBuffers;
std::vector<VulkanSemaphore> imageAvailableSemaphores;
std::vector<VulkanSemaphore> renderFinishedSemaphores;
std::vector<VulkanFence> queueFences;
Core::Array<VkCommandBuffer> mainDrawBuffers;
Core::Array<VkCommandBuffer> guiDrawBuffers;

VulkanBuffer VertexBuffer;
VulkanBuffer IndexBuffer;

VulkanImage TextureImage;
VulkanImageView TextureView;
VulkanSampler TextureSampler;


const std::vector<std::string> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const std::vector<std::string> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
const int MAX_FRAME_IN_FLIGHT                   = 2;
size_t currentFrame                             = 0;
bool framebufferResized                         = false;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding                         = 0;
        bindingDescription.stride                          = sizeof(Vertex);
        bindingDescription.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos);

        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(Vertex, color);

        attributeDescriptions[2].binding  = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset   = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

std::vector<uint32_t> indices;
std::vector<Vertex> vertices;

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

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
    queues.graphics.pool.freeBuffers(device, commandBuffers);
    VulkanGraphicsPipeline::Destroy(device, graphicsPipeline);
    VulkanRenderPass::Destroy(device, renderPass);
    VulkanSwapChain::Destroy(device, swapChain);
}

void cleanupVulkan() {
    vkDeviceWaitIdle(device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for(int i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        VulkanSemaphore::Destroy(device, renderFinishedSemaphores[i]);
        VulkanSemaphore::Destroy(device, imageAvailableSemaphores[i]);
        VulkanFence::Destroy(device, queueFences[i]);
    }

    cleanupSwapChain();

    VulkanImageView::Destroy(device, TextureView);
    VulkanSampler::Destroy(device, TextureSampler);

    physicalDevice.DestroyImage(device, TextureImage);
    physicalDevice.DestroyBuffer(device, VertexBuffer);
    physicalDevice.DestroyBuffer(device, IndexBuffer);

    VulkanPipelineLayout::Destroy(device, pipelineLayout);

    VulkanDescriptorPool::Destroy(device, descriptorPool);

    for(size_t i = 0; i < uniformBuffers.size(); i++) {
        physicalDevice.DestroyBuffer(device, uniformBuffers[i]);
    }

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    VulkanQueues::Destroy(device, queues);
    VulkanLogicalDevice::Destroy(device);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    VulkanInstance::Destroy(instance);
}

void createGraphicsPipeline() {
    VulkanShaderModule vertexShader   = VulkanShaderModule::CreateFromFile(device, "Shaders/triangle/vert.spv");
    VulkanShaderModule fragmentShader = VulkanShaderModule::CreateFromFile(device, "Shaders/triangle/frag.spv");

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
    uint32_t swapChainCount = static_cast<uint32_t>(swapChain.framebuffers.size());

    commandBuffers  = queues.graphics.pool.allocateBuffers(device, swapChainCount, VulkanCommandBufferLevel::Secondary);
    mainDrawBuffers = queues.graphics.pool.allocateBuffers(device, swapChainCount);
    guiDrawBuffers  = queues.graphics.pool.allocateBuffers(device, swapChainCount, VulkanCommandBufferLevel::Secondary);

    descriptorPool = VulkanDescriptorPool::Create(
          device, swapChainCount + 1, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
    uniformBuffersDescriptors =
          descriptorPool.allocateSets(device, static_cast<uint32_t>(commandBuffers.size()), descriptorSetLayout);

    uniformBuffers.resize(commandBuffers.size());

    for(size_t i = 0; i < commandBuffers.size(); i++) {
        uniformBuffers[i] =
              physicalDevice.createBuffer(device, sizeof(UniformBufferObject), VulkanBufferUsageType::Uniform);

        VulkanDescriptorSetUpdate update;
        update.addBuffer(0, uniformBuffers[i]);
        update.addSampledImage(1, TextureView, TextureSampler);
        update.update(device, uniformBuffersDescriptors[i]);

        VkCommandBufferInheritanceInfo inheritance = {};
        inheritance.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritance.renderPass                     = renderPass;
        inheritance.subpass                        = 0;
        inheritance.framebuffer                    = swapChain.framebuffers[i];

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags =
              VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        beginInfo.pInheritanceInfo = &inheritance;    // Optional

        VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        vkCmdSetViewport(commandBuffers[i], 0, 1, &swapChain.viewport);

        VkBuffer vertexBuffers[] = {VertexBuffer};
        VkDeviceSize offsets[]   = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[i], IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffers[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0,
                                1,
                                &uniformBuffersDescriptors[i],
                                0,
                                nullptr);
        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
    }
}

VkCommandBuffer copyBuffer(VulkanBuffer& source, VulkanBuffer& destination) {
    VkCommandBuffer copyBuffer = queues.transfer.pool.allocateSingleUseBuffer(device);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset    = 0;    // Optional
    copyRegion.dstOffset    = 0;    // Optional
    copyRegion.size         = source.size;
    vkCmdCopyBuffer(copyBuffer, source, destination, 1, &copyRegion);
    vkEndCommandBuffer(copyBuffer);

    queues.transfer.submit(copyBuffer, VulkanQueueSubmitType::Transfer);
    return copyBuffer;
}

void createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding                      = 0;
    uboLayoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount              = 1;
    uboLayoutBinding.stageFlags                   = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers           = nullptr;    // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding                      = 1;
    samplerLayoutBinding.descriptorType               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount              = 1;
    samplerLayoutBinding.stageFlags                   = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers           = nullptr;    // Optional

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount                    = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings                       = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout));
}

void createVertexBuffer() {
    std::string file = "Models/chalet.mesh";
    // std::string file = "Models/rungholt.obj";
    // std::string file = "Models/powerplant.obj";
    // std::string file = "Models/san-miguel.obj";

    std::chrono::steady_clock clock;
    auto now = clock.now();

    ASSIGN_OR_ASSERT(Core::IO::InputStream input, Core::IO::OpenFileForRead(file));

    auto model = input.read<Assets::Mesh>();
    auto end   = clock.now();

    std::chrono::duration<float> d = end - now;
    Core::Log::Info(Test, "Took {}s to load model.", d.count());


    now = clock.now();

    model.deduplicateVertices();
    uint32_t positionOffset = model.vertexFormat.properties[Assets::VertexUsage::POSITION].byteOffset;
    uint32_t textureOffset  = model.vertexFormat.properties[Assets::VertexUsage::TEXTURE].byteOffset;
    uint32_t indexOffset    = static_cast<uint32_t>(vertices.size());

    for(Core::IO::BufferViewWindow view(model.vertexData, model.vertexFormat.byteCount()); view; ++view) {
        Vertex vertex = {};

        vertex.pos      = view.read<glm::vec3>(positionOffset);
        vertex.texCoord = view.read<glm::vec2>(textureOffset);
        vertex.color    = {1.0f, 1.0f, 1.0f};

        vertex.texCoord.y = 1.0f - vertex.texCoord.y;

        vertices.push_back(vertex);
    }

    indices = model.indexData;

    end                 = clock.now();
    d                   = end - now;
    uint32_t vertexSize = static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
    uint32_t indexSize  = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));

    Core::Log::Info(
          Test, "Took {}s to create vertex data ({} verts, {} indices).", d.count(), vertices.size(), indices.size());

    VulkanBuffer vertexStagingBuffer = physicalDevice.createBuffer(device,
                                                                   vertexSize,
                                                                   VulkanBufferUsageType::None,
                                                                   VulkanBufferTransferType::Source,
                                                                   VulkanMemoryVisibility::Host);

    vertexStagingBuffer.upload(device, vertices.data(), vertexStagingBuffer.size);

    VertexBuffer = physicalDevice.createBuffer(device,
                                               vertexSize,
                                               VulkanBufferUsageType::Vertex,
                                               VulkanBufferTransferType::Destination,
                                               VulkanMemoryVisibility::Device);

    VkCommandBuffer vertexCopyBuffer = copyBuffer(vertexStagingBuffer, VertexBuffer);


    VulkanBuffer indexStagingBuffer = physicalDevice.createBuffer(device,
                                                                  indexSize,
                                                                  VulkanBufferUsageType::None,
                                                                  VulkanBufferTransferType::Source,
                                                                  VulkanMemoryVisibility::Host);

    indexStagingBuffer.upload(device, indices.data(), indexStagingBuffer.size);

    IndexBuffer = physicalDevice.createBuffer(device,
                                              indexSize,
                                              VulkanBufferUsageType::Index,
                                              VulkanBufferTransferType::Destination,
                                              VulkanMemoryVisibility::Device);

    VkCommandBuffer indexCopyBuffer = copyBuffer(indexStagingBuffer, IndexBuffer);

    vkQueueWaitIdle(queues.transfer);
    queues.transfer.pool.freeBuffers(device, {vertexCopyBuffer, indexCopyBuffer});
    physicalDevice.DestroyBuffer(device, vertexStagingBuffer);
    physicalDevice.DestroyBuffer(device, indexStagingBuffer);
}
void copyBufferToImage(VkCommandBuffer commandBuffer, VulkanBuffer& buffer, VulkanImage image) {
    VkBufferImageCopy region = {};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = image.extent;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void createTextureImage() {
    std::string file = "Textures/chalet.jpg";
    ASSIGN_OR_ASSERT(Core::Array<std::byte> imageData, Core::IO::ReadBinaryFile(file))

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels        = stbi_load_from_memory(reinterpret_cast<unsigned char*>(imageData.data()),
                                            imageData.size(),
                                            &texWidth,
                                            &texHeight,
                                            &texChannels,
                                            STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if(pixels == nullptr) {
        Core::Log::Error(Test, "Failed to load texture.");
        return;
    }

    VulkanBuffer transferBuffer =
          physicalDevice.createBuffer(device, imageSize, VulkanBufferUsageType::None, VulkanBufferTransferType::Source);
    transferBuffer.upload(device, pixels, imageSize);

    stbi_image_free(pixels);

    TextureImage = physicalDevice.createImage(device,
                                              {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)},
                                              VK_FORMAT_R8G8B8A8_UNORM,
                                              VulkanImageUsageType::Sampled,
                                              VulkanImageTransferType::Destination,
                                              VulkanMemoryVisibility::Device);

    VkCommandBuffer commandBuffer = queues.transfer.pool.allocateSingleUseBuffer(device);

    TextureImage.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(commandBuffer, transferBuffer, TextureImage);
    TextureImage.transitionLayout(
          commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkEndCommandBuffer(commandBuffer);
    queues.transfer.submit(commandBuffer, VulkanQueueSubmitType::Transfer);

    vkQueueWaitIdle(queues.transfer);
    queues.transfer.pool.freeBuffers(device, {commandBuffer});
    physicalDevice.DestroyBuffer(device, transferBuffer);

    TextureSampler = VulkanSampler::Create(device);
    TextureView    = VulkanImageView::Create(device, TextureImage, TextureImage.format);
}

Core::Status initVulkan(GLFWwindow* window) {
    instance = VulkanInstance::Create("Test", getRequiredExtensions(), validationLayers);

    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));

    ASSIGN_OR_RETURN(physicalDevice,
                     VulkanPhysicalDevice::Find(instance, surface, deviceExtensions, GetWindowSize(window)));

    device = VulkanLogicalDevice::Create(physicalDevice.queueIndices, deviceExtensions, validationLayers);
    queues = VulkanQueues::Create(device, physicalDevice.queueIndices);

    renderPass = VulkanRenderPass::Create(
          device, physicalDevice.swapChainDetails.format.format, physicalDevice.swapChainDetails.depthFormat);
    swapChain = VulkanSwapChain::Create(device, physicalDevice, queues, renderPass);

    createDescriptorSetLayout();
    pipelineLayout = VulkanPipelineLayout::Create(device, {descriptorSetLayout});


    createGraphicsPipeline();

    createVertexBuffer();
    createTextureImage();

    createCommandBuffers();

    imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    queueFences.resize(MAX_FRAME_IN_FLIGHT);
    for(int i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        imageAvailableSemaphores[i] = VulkanSemaphore::Create(device);
        renderFinishedSemaphores[i] = VulkanSemaphore::Create(device);
        queueFences[i]              = VulkanFence::Create(device, VulkanFenceState::Signaled);
    }


    return Core::Status::Ok();
}

void recreateSwapChain(GLFWwindow* window) {
    int width = 0, height = 0;
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);
    cleanupSwapChain();

    physicalDevice.swapChainDetails = VulkanSwapChainDetails::Find(physicalDevice, surface, GetWindowSize(window));

    renderPass = VulkanRenderPass::Create(
          device, physicalDevice.swapChainDetails.format.format, physicalDevice.swapChainDetails.depthFormat);
    swapChain = VulkanSwapChain::Create(device, physicalDevice, queues, renderPass);
    createGraphicsPipeline();
    createCommandBuffers();
}

bool windowShowned = true;
VkCommandBuffer RenderGUI(uint32_t framebufferIndex) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow(&windowShowned);
    VkCommandBuffer MainBuffer = mainDrawBuffers[framebufferIndex];
    VK_CHECK(vkResetCommandBuffer(MainBuffer, 0));

    VkCommandBuffer GuiCommandBuffer = guiDrawBuffers[framebufferIndex];
    VK_CHECK(vkResetCommandBuffer(GuiCommandBuffer, 0));

    VkCommandBufferInheritanceInfo inheritance = {};
    inheritance.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance.renderPass                     = renderPass;
    inheritance.subpass                        = 0;
    inheritance.framebuffer                    = swapChain.framebuffers[framebufferIndex];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo = &inheritance;    // Optional

    VK_CHECK(vkBeginCommandBuffer(GuiCommandBuffer, &beginInfo));
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GuiCommandBuffer);
    VK_CHECK(vkEndCommandBuffer(GuiCommandBuffer));

    VkCommandBufferBeginInfo beginInfo2 = {};
    beginInfo2.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo2.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(MainBuffer, &beginInfo2));

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass            = renderPass;
    renderPassInfo.framebuffer           = swapChain.framebuffers[framebufferIndex];
    renderPassInfo.renderArea.offset     = {0, 0};
    renderPassInfo.renderArea.extent     = swapChain.extent;

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color                    = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil             = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues    = clearValues.data();

    vkCmdBeginRenderPass(MainBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdExecuteCommands(MainBuffer, 1, &commandBuffers[framebufferIndex]);
    vkCmdExecuteCommands(MainBuffer, 1, &GuiCommandBuffer);
    vkCmdEndRenderPass(MainBuffer);

    VK_CHECK(vkEndCommandBuffer(MainBuffer));

    return MainBuffer;
}

void drawFrame(GLFWwindow* window, Core::Clock::Seconds delta) {
    static UniformBufferObject ubo = {
          glm::mat4(1.0f),
          glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1)),
          glm::perspective(glm::radians(45.0f),
                           swapChain.extent.width / static_cast<float>(swapChain.extent.height),
                           0.1f,
                           10000.0f)};

    auto acquisitionResult = swapChain.acquireNextImage(device, imageAvailableSemaphores[currentFrame]);

    if(acquisitionResult.result == VK_ERROR_OUT_OF_DATE_KHR || acquisitionResult.result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain(window);
        return;
    } else if(acquisitionResult.result != VK_SUCCESS) {
        Core::Log::Error(Test, "Couldn't acquire swap chain image: {}", acquisitionResult.result);
        return;
    }

    queueFences[currentFrame].waitAndReset(device);

    float rads  = glm::radians(90.0f);
    float angle = delta.count() * rads;

    ubo.model = glm::rotate(ubo.model, angle, glm::vec3(0, 0, 1));
    uniformBuffers[acquisitionResult.imageIndex].upload(device, &ubo, sizeof(ubo));


    VkCommandBuffer buffer = RenderGUI(acquisitionResult.imageIndex);

    queues.graphics.submit(buffer,
                           VulkanQueueSubmitType::Graphics,
                           imageAvailableSemaphores[currentFrame],
                           renderFinishedSemaphores[currentFrame],
                           queueFences[currentFrame]);

    VkResult presentResult =
          queues.present.present(swapChain, renderFinishedSemaphores[currentFrame], acquisitionResult.imageIndex);

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;

    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain(window);
    } else if(presentResult != VK_SUCCESS) {
        Core::Log::Error(Test, "Failed to present image: {}", presentResult);
    }
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
    using namespace std::chrono_literals;

    Core::LogManager::SetCategoryLevel(Vulkan, Core::LogLevel::Trace);

    Core::IO::DefaultFileSystem.addMount(new Core::IO::VirtualFileSystemMount("Shaders", "test_app/Content/Shaders/"));
    Core::IO::DefaultFileSystem.addMount(new Core::IO::VirtualFileSystemMount("Models", "test_app/Content/Models/"));
    Core::IO::DefaultFileSystem.addMount(
          new Core::IO::VirtualFileSystemMount("Textures", "test_app/Content/Textures/"));

    GLFWwindow* window = initWindow();

    Core::Status initStatus = initVulkan(window);
    if(initStatus.isError()) {
        Core::Log::Error(Test, "Failed to initialize vulkan: {}", initStatus.message());
        return EXIT_FAILURE;
    }

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = instance;
    init_info.PhysicalDevice            = physicalDevice;
    init_info.Device                    = device;
    init_info.QueueFamily               = queues.graphics.familyIndex;
    init_info.Queue                     = queues.graphics;
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.DescriptorPool            = descriptorPool;
    init_info.Allocator                 = nullptr;
    init_info.CheckVkResultFn           = VK_NULL_HANDLE;
    ImGui_ImplVulkan_Init(&init_info, renderPass);

    ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();

    VkCommandBuffer buffer = queues.graphics.pool.allocateSingleUseBuffer(device);
    ImGui_ImplVulkan_CreateFontsTexture(buffer);
    VK_CHECK(vkEndCommandBuffer(buffer));
    queues.graphics.submit(buffer, VulkanQueueSubmitType::Transfer);
    vkQueueWaitIdle(queues.graphics);

    Core::SystemClockTicker ticker;
    Core::Clock clock;
    Core::Clock clock2;
    Core::Timer timer(&clock2);

    ticker.registerClock(&clock);
    ticker.registerClock(&clock2);

    double changePerSecond = -0.45;
    Core::Clock::Seconds period(2.0f);

    uint64_t frameCount = 0;

    Core::Clock::Seconds lastFPSCalculation = clock.totalElapsedSeconds();

    while(!glfwWindowShouldClose(window)) {
        frameCount++;
        ticker.tick();
        glfwPollEvents();
        drawFrame(window, clock.tickedTime());

        // clock.timeScale += changePerSecond * clock2.tickedSeconds().count();

        if(timer.elapsedTime() > period) {
            changePerSecond *= -1;
            timer.reset();
        }

        if(frameCount == 100) {
            Core::Clock::Seconds now = clock.totalElapsedSeconds();

            // Core::Log::Info(Test, "{}ms", (now - lastFPSCalculation).count() / frameCount);
            frameCount         = 0;
            lastFPSCalculation = now;
        }
    }


    Core::Clock::Seconds elapsedTime = clock.totalElapsedTime();
    Core::Log::Info(Test, "Elapsed time: {:f}s", elapsedTime.count());

    cleanupVulkan();

    glfwDestroyWindow(window);

    glfwTerminate();

    return EXIT_SUCCESS;
}