// Test.cpp : This file contains the 'main' function. Program execution begins
// and ends there.
//
#include <Core/Logging/Logger.h>

#define GLFW_INCLUDE_NONE
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


#include <Core/IO/Serialization/BufferView.h>

#include <Assets/Materials/Shader.h>

#include <GUI/Window.h>

#include <iostream>

#include <Renderer/Backends/Vulkan/DiagnosticCheckpoint.h>
#include <Renderer/Backends/Vulkan/VulkanRendererBackend.h>

#include <Renderer/Backends/Vulkan/VulkanCommandPool.h>
#include <Renderer/Backends/Vulkan/VulkanDescriptorPool.h>
#include <Renderer/Backends/Vulkan/VulkanEnums.h>
#include <Renderer/Backends/Vulkan/VulkanFence.h>
#include <Renderer/Backends/Vulkan/VulkanGraphicsPipeline.h>
#include <Renderer/Backends/Vulkan/VulkanImage.h>
#include <Renderer/Backends/Vulkan/VulkanInstance.h>
#include <Renderer/Backends/Vulkan/VulkanLogicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPhysicalDevice.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineLayout.h>
#include <Renderer/Backends/Vulkan/VulkanPipelineShaderStage.h>
#include <Renderer/Backends/Vulkan/VulkanQueue.h>
#include <Renderer/Backends/Vulkan/VulkanSampler.h>
#include <Renderer/Backends/Vulkan/VulkanSemaphore.h>
#include <Renderer/Backends/Vulkan/VulkanShaderModule.h>
#include <Renderer/Backends/Vulkan/VulkanSwapChain.h>


#include <Renderer/Resources/GPUMesh.h>
#include <Renderer/Resources/GPUTexture.h>

#include <absl/container/flat_hash_map.h>

#include <Assets/Importers/OBJImporter.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <atomic>
#include <sstream>


Core::LogCategory Test("Test");
using namespace Renderer::Backends::Vulkan;

VulkanDescriptorPool descriptorPool;
VulkanSwapChain swapChain;
VulkanGraphicsPipeline graphicsPipeline;
Core::Array<VkDescriptorSet> uniformBuffersDescriptors;
Core::Array<VulkanBuffer> uniformBuffers;
Core::Array<VkCommandBuffer> commandBuffers;
Core::Array<VulkanSemaphore> imageAvailableSemaphores;
Core::Array<VulkanSemaphore> renderFinishedSemaphores;
Core::Array<VulkanFence> queueFences;
Core::Array<VkCommandBuffer> mainDrawBuffers;
Core::Array<VkCommandBuffer> guiDrawBuffers;

Renderer::Resources::GPUMesh mesh;
Renderer::Resources::GPUTexture texture;

const int MAX_FRAME_IN_FLIGHT = 2;
size_t currentFrame           = 0;

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

void cleanupVulkan(VulkanRendererBackend& backend) {
    VulkanLogicalDevice& device          = backend.getLogicalDevice();
    VulkanPhysicalDevice& physicalDevice = backend.getPhysicalDevice();
    VulkanQueues& queues                 = backend.getQueues();

    vkDeviceWaitIdle(device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    for(int i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        VulkanSemaphore::Destroy(device, renderFinishedSemaphores[i]);
        VulkanSemaphore::Destroy(device, imageAvailableSemaphores[i]);
        VulkanFence::Destroy(device, queueFences[i]);
    }

    VulkanSwapChain::Destroy(device, swapChain);

    queues.graphics.pool.freeBuffers(device, commandBuffers);
    queues.graphics.pool.freeBuffers(device, mainDrawBuffers);
    queues.graphics.pool.freeBuffers(device, guiDrawBuffers);

    VulkanGraphicsPipeline::Destroy(device, graphicsPipeline);

    VulkanImageView::Destroy(device, texture.view);
    VulkanSampler::Destroy(device, texture.sampler);

    physicalDevice.DestroyImage(device, texture.image);
    physicalDevice.DestroyBuffer(device, mesh.vertexBuffer);
    physicalDevice.DestroyBuffer(device, mesh.indexBuffer);

    VulkanDescriptorPool::Destroy(device, descriptorPool);

    for(size_t i = 0; i < uniformBuffers.count(); i++) {
        physicalDevice.DestroyBuffer(device, uniformBuffers[i]);
    }

    backend.shutdown();
}

Core::Status createGraphicsPipeline(VulkanRendererBackend& backend, const Assets::Mesh& meshData) {
    VulkanLogicalDevice& device = backend.getLogicalDevice();
    VulkanRenderPass renderPass = *backend.getSwapChainRenderPass();

    ASSIGN_OR_RETURN(Core::IO::InputStream input, Core::IO::OpenFileForRead("Shaders/triangle.shader"));
    Assets::Shader shader = input.read<Assets::Shader>();

    graphicsPipeline = VulkanGraphicsPipeline::Create(device, shader, meshData.vertexFormat, renderPass);

    return Core::Status::Ok();
}

void recordCommandBuffers(VulkanRendererBackend& backend) {
    for(size_t i = 0; i < commandBuffers.count(); i++) {
        VK_CHECK(vkResetCommandBuffer(commandBuffers[i], 0));

        VkCommandBufferInheritanceInfo inheritance = {};
        inheritance.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritance.renderPass                     = *backend.getSwapChainRenderPass();
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
        vkCmdSetScissor(commandBuffers[i], 0, 1, &swapChain.scissor);

        VkBuffer vertexBuffers[] = {mesh.vertexBuffer};
        VkDeviceSize offsets[]   = {mesh.vertexBufferOffset};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[i], mesh.indexBuffer, mesh.indexBufferOffset, mesh.indexType);
        vkCmdBindDescriptorSets(commandBuffers[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                graphicsPipeline.pipelineLayout,
                                0,
                                1,
                                &uniformBuffersDescriptors[i],
                                0,
                                nullptr);
        vkCmdDrawIndexed(commandBuffers[i], mesh.indexCount, 1, 0, 0, 0);

        VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
    }
}

void createCommandBuffers(VulkanRendererBackend& backend) {
    VulkanLogicalDevice& device          = backend.getLogicalDevice();
    VulkanPhysicalDevice& physicalDevice = backend.getPhysicalDevice();
    VulkanQueues& queues                 = backend.getQueues();

    uint32_t swapChainCount = swapChain.framebuffers.size();

    commandBuffers  = queues.graphics.pool.allocateBuffers(device, swapChainCount, VulkanCommandBufferLevel::Secondary);
    mainDrawBuffers = queues.graphics.pool.allocateBuffers(device, swapChainCount);
    guiDrawBuffers  = queues.graphics.pool.allocateBuffers(device, swapChainCount, VulkanCommandBufferLevel::Secondary);

    descriptorPool = VulkanDescriptorPool::Create(
          device, swapChainCount + 1, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
    uniformBuffersDescriptors =
          descriptorPool.allocateSets(device, commandBuffers.count(), graphicsPipeline.descriptorSetLayout);

    uniformBuffers.ensureCapacity(commandBuffers.count());

    for(uint64_t i = 0; i < commandBuffers.count(); i++) {
        uniformBuffers.emplace(
              physicalDevice.createBuffer(device, sizeof(UniformBufferObject), VulkanBufferUsageType::Uniform));

        VulkanDescriptorSetUpdate update;
        update.addBuffer(0, uniformBuffers[i]);
        update.addSampledImage(1, texture.view, texture.sampler);
        update.update(device, uniformBuffersDescriptors[i]);
    }

    recordCommandBuffers(backend);
}


Core::Status createVertexBuffer(VulkanRendererBackend& backend) {
    std::string file = "Models/chalet.mesh";

    ASSIGN_OR_RETURN(Core::IO::InputStream input, Core::IO::OpenFileForRead(file));

    auto model = input.read<Assets::Mesh>();
    RETURN_IF_ERROR(createGraphicsPipeline(backend, model));

    mesh = backend.createMesh(model.vertexData, model.indexData, VK_INDEX_TYPE_UINT32);

    return Core::Status::Ok();
}

Core::Status createTextureImage(VulkanRendererBackend& backend) {
    VulkanLogicalDevice& device = backend.getLogicalDevice();

    std::string file = "Textures/chalet.jpg";
    ASSIGN_OR_RETURN(Core::Array<std::byte> imageData, Core::IO::ReadBinaryFile(file));

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<unsigned char*>(imageData.rawData()),
                                            imageData.count(),
                                            &texWidth,
                                            &texHeight,
                                            &texChannels,
                                            STBI_rgb_alpha);
    if(pixels == nullptr) {
        return Core::Status::Error("Failed to load texture '{}'", file);
    }

    std::span<const std::byte> data(reinterpret_cast<const std::byte*>(pixels), texWidth * texHeight * 4);

    texture = backend.createTexture(
          data, VK_FORMAT_R8G8B8A8_UNORM, {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)});

    stbi_image_free(pixels);

    return Core::Status::Ok();
}

Core::Status initVulkanBackend(VulkanRendererBackend& backend, GUI::Window& window) {
    swapChain = backend.makeSwapChain(window.getSize());

    RETURN_IF_ERROR(createVertexBuffer(backend));
    RETURN_IF_ERROR(createTextureImage(backend));

    vkQueueWaitIdle(backend.getQueues().transfer);
    backend.processFinishedSubmitResources();

    createCommandBuffers(backend);

    imageAvailableSemaphores.ensureCapacity(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.ensureCapacity(MAX_FRAME_IN_FLIGHT);
    queueFences.ensureCapacity(MAX_FRAME_IN_FLIGHT);
    for(int i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        imageAvailableSemaphores.emplace(VulkanSemaphore::Create(backend.getLogicalDevice()));
        renderFinishedSemaphores.emplace(VulkanSemaphore::Create(backend.getLogicalDevice()));
        queueFences.emplace(VulkanFence::Create(backend.getLogicalDevice(), VulkanFenceState::Signaled));
    }

    return Core::Status::Ok();
}

void recreateSwapChain(GUI::Window& window, VulkanRendererBackend& backend) {
    int width = 0, height = 0;
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(window.getHandle(), &width, &height);
        glfwWaitEvents();
    }

    Core::Log::Info(Test, "Resizing swapchain to {}x{}", width, height);

    swapChain = backend.makeSwapChain(window.getSize(), swapChain);

    recordCommandBuffers(backend);
    vkDeviceWaitIdle(backend.getLogicalDevice());
}

bool windowShowned = true;
VkCommandBuffer RenderGUI(uint32_t framebufferIndex, VulkanRendererBackend& backend) {
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
    inheritance.renderPass                     = *backend.getSwapChainRenderPass();
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
    renderPassInfo.renderPass            = *backend.getSwapChainRenderPass();
    renderPassInfo.framebuffer           = swapChain.framebuffers[framebufferIndex];
    renderPassInfo.renderArea            = swapChain.scissor;

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

void drawFrame(GUI::Window& window, Core::Clock::Seconds delta, VulkanRendererBackend& backend) {
    VulkanLogicalDevice& device = backend.getLogicalDevice();
    VulkanQueues& queues        = backend.getQueues();

    static UniformBufferObject ubo = {
          glm::mat4(1.0f),
          glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1)),
          glm::perspective(glm::radians(45.0f),
                           swapChain.viewport.width / static_cast<float>(swapChain.viewport.height),
                           0.1f,
                           10000.0f)};

    if(window.hasResized()) {
        Core::Log::Info(Test, "Recreating swap chain");
        recreateSwapChain(window, backend);
        ubo.projection = glm::perspective(glm::radians(45.0f),
                                          swapChain.viewport.width / static_cast<float>(swapChain.viewport.height),
                                          0.1f,
                                          10000.0f);
    }

    auto acquisitionResult = swapChain.acquireNextImage(device, imageAvailableSemaphores[currentFrame]);

    if(acquisitionResult.result == VK_ERROR_OUT_OF_DATE_KHR) {
        Core::Log::Error(Test, "Out of date swap chain");
        return;
    } else if(acquisitionResult.result != VK_SUCCESS && acquisitionResult.result != VK_SUBOPTIMAL_KHR) {
        Core::Log::Error(Test, "Couldn't acquire swap chain image: {}", acquisitionResult.result);
        return;
    }
    window.clearResized();

    ASSERT_WITH_MESSAGE(queueFences[currentFrame].waitAndReset(device), "Failed to wait for queue fence!");

    float rads  = glm::radians(90.0f);
    float angle = delta.count() * rads;

    ubo.model = glm::rotate(ubo.model, angle, glm::vec3(0, 0, 1));
    uniformBuffers[acquisitionResult.imageIndex].upload(device, Core::ToBytes(ubo));

    VkCommandBuffer buffer = RenderGUI(acquisitionResult.imageIndex, backend);

    queues.graphics.submit(buffer,
                           VulkanQueueSubmitType::Graphics,
                           imageAvailableSemaphores[currentFrame],
                           renderFinishedSemaphores[currentFrame],
                           queueFences[currentFrame]);

    VkResult presentResult =
          queues.present.present(swapChain, renderFinishedSemaphores[currentFrame], acquisitionResult.imageIndex);

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;

    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        Core::Log::Info(Test, "Not recreating swap chain");
        // recreateSwapChain(window);
    } else if(presentResult != VK_SUCCESS) {
        Core::Log::Error(Test, "Failed to present image: {}", presentResult);
    }
}

GLFWwindow* initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    return glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

Core::Status run() {
    using namespace std::chrono_literals;
    using namespace Renderer::Backends::Vulkan;

    Core::LogManager::SetCategoryLevel(Vulkan, Core::LogLevel::Trace);

    Core::IO::DefaultFileSystem.addMount(new Core::IO::VirtualFileSystemMount("Shaders", "content/Shaders/"));
    Core::IO::DefaultFileSystem.addMount(new Core::IO::VirtualFileSystemMount("Models", "content/Models/"));
    Core::IO::DefaultFileSystem.addMount(new Core::IO::VirtualFileSystemMount("Textures", "content/Textures/"));

    CONSTRUCT_OR_RETURN(auto window, GUI::Window::Create("Vulkan", 800, 600));

    CONSTRUCT_OR_RETURN(
          VulkanRendererBackend backend,
          VulkanRendererBackend::CreateWithSurface("Test",
                                                   [&](auto instance) { return window->createSurface(instance); },
                                                   window->getRequiredVulkanExtensions(),
                                                   {},
                                                   {}));

    RETURN_IF_ERROR(initVulkanBackend(backend, *window));

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForVulkan(window->getHandle(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = backend.getInstance();
    init_info.PhysicalDevice            = backend.getPhysicalDevice();
    init_info.Device                    = backend.getLogicalDevice();
    init_info.QueueFamily               = backend.getQueues().graphics.familyIndex;
    init_info.Queue                     = backend.getQueues().graphics;
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.MinImageCount             = swapChain.images.size();
    init_info.ImageCount                = swapChain.images.size();
    init_info.DescriptorPool            = descriptorPool;
    init_info.Allocator                 = nullptr;
    init_info.CheckVkResultFn           = [](VkResult err) { VK_CHECK(err); };
    ImGui_ImplVulkan_Init(&init_info, *backend.getSwapChainRenderPass());

    ImGui::StyleColorsDark();
    io.Fonts->AddFontDefault();

    VulkanQueue& transferQueue = backend.getQueues().transfer;
    VkCommandBuffer buffer     = transferQueue.pool.allocateSingleUseBuffer(backend.getLogicalDevice());
    ImGui_ImplVulkan_CreateFontsTexture(buffer);
    VK_CHECK(vkEndCommandBuffer(buffer));
    transferQueue.submit(buffer, VulkanQueueSubmitType::Transfer);
    vkQueueWaitIdle(transferQueue);
    transferQueue.pool.freeBuffers(backend.getLogicalDevice(), {buffer});

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

    while(!window->shouldClose()) {
        frameCount++;
        ticker.tick();
        glfwPollEvents();
        drawFrame(*window, clock.tickedTime(), backend);

        // clock.timeScale += changePerSecond * clock2.tickedSeconds().count();

        if(timer.elapsedTime() > period) {
            changePerSecond *= -1;
            timer.reset();
        }

        if(frameCount == 100) {
            Core::Clock::Seconds now = clock.totalElapsedSeconds();

            // Core::Log::Info(Test, "{}ms", (now - lastFPSCalculation).count() /
            // frameCount);
            frameCount         = 0;
            lastFPSCalculation = now;
        }
    }

    Core::Clock::Seconds elapsedTime = clock.totalElapsedTime();
    Core::Log::Info(Test, "Elapsed time: {:f}s", elapsedTime.count());

    cleanupVulkan(backend);

    return Core::Status::Ok();
}

int main() {
    Core::Status status = run();
    if(status.isError()) {
        Core::Log::Error(Test, status.message());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}