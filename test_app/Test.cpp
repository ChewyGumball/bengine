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

#include <Assets/Models/Mesh.h>
#include <Assets/Textures/Texture.h>

#include <imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#include <atomic>
#include <sstream>


Core::LogCategory Test("Test");
using namespace Renderer::Backends::Vulkan;

VulkanDescriptorPool descriptorPool;
VulkanGraphicsPipeline graphicsPipeline;
Core::Array<VkDescriptorSet> uniformBuffersDescriptors;
Core::Array<VulkanBuffer> uniformBuffers;
Core::Array<VulkanBuffer> instanceBuffers;
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
    glm::mat4 view;
    glm::mat4 projection;
};

struct InstanceBufferObject {
    glm::mat4 model;
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

    queues.graphics.pool.freeBuffers(device, commandBuffers);
    queues.graphics.pool.freeBuffers(device, mainDrawBuffers);
    queues.graphics.pool.freeBuffers(device, guiDrawBuffers);

    VulkanGraphicsPipeline::Destroy(device, graphicsPipeline);

    VulkanImageView::Destroy(device, texture.view);
    VulkanSampler::Destroy(device, texture.sampler);

    VulkanImage::Destroy(texture.image);
    VulkanBuffer::Destroy(mesh.vertexBuffer);
    VulkanBuffer::Destroy(mesh.indexBuffer);

    VulkanDescriptorPool::Destroy(device, descriptorPool);

    for(size_t i = 0; i < uniformBuffers.count(); i++) {
        VulkanBuffer::Destroy(uniformBuffers[i]);
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

void recordCommandBuffers(uint32_t framebufferIndex, VulkanRendererBackend& backend) {
    VkCommandBuffer commandBuffer = commandBuffers[framebufferIndex];

    VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

    VulkanSwapChain& swapChain = *backend.getSwapChain();

    VkCommandBufferInheritanceInfo inheritance = {};
    inheritance.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance.renderPass                     = *backend.getSwapChainRenderPass();
    inheritance.subpass                        = 0;
    inheritance.framebuffer                    = swapChain.framebuffers[framebufferIndex];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo         = &inheritance;    // Optional

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdSetViewport(commandBuffer, 0, 1, &swapChain.viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &swapChain.scissor);

    VkBuffer vertexBuffers[] = {mesh.vertexBuffer, instanceBuffers[framebufferIndex]};
    VkDeviceSize offsets[]   = {mesh.vertexBufferOffset, 0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, mesh.indexBufferOffset, mesh.indexType);
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            graphicsPipeline.pipelineLayout,
                            0,
                            1,
                            &uniformBuffersDescriptors[framebufferIndex],
                            0,
                            nullptr);
    vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 2, 0, 0, 0);

    VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

void createCommandBuffers(VulkanRendererBackend& backend) {
    VulkanLogicalDevice& device          = backend.getLogicalDevice();
    VulkanPhysicalDevice& physicalDevice = backend.getPhysicalDevice();
    VulkanQueues& queues                 = backend.getQueues();
    VulkanSwapChain& swapChain           = *backend.getSwapChain();

    uint32_t swapChainCount = swapChain.framebuffers.size();

    commandBuffers  = queues.graphics.pool.allocateBuffers(device, swapChainCount, VulkanCommandBufferLevel::Secondary);
    mainDrawBuffers = queues.graphics.pool.allocateBuffers(device, swapChainCount);
    guiDrawBuffers  = queues.graphics.pool.allocateBuffers(device, swapChainCount, VulkanCommandBufferLevel::Secondary);

    descriptorPool = VulkanDescriptorPool::Create(
          device, swapChainCount + 1, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
    uniformBuffersDescriptors =
          descriptorPool.allocateSets(device, commandBuffers.count(), graphicsPipeline.descriptorSetLayout);

    instanceBuffers.ensureCapacity(commandBuffers.count());
    uniformBuffers.ensureCapacity(commandBuffers.count());


    for(uint64_t i = 0; i < commandBuffers.count(); i++) {
        uniformBuffers.emplace(backend.createBuffer(sizeof(UniformBufferObject), VulkanBufferUsageType::Uniform));

        VulkanDescriptorSetUpdate update;
        update.addBuffer(0, uniformBuffers[i]);
        update.addSampledImage(1, texture.view, texture.sampler);
        update.update(device, uniformBuffersDescriptors[i]);

        instanceBuffers.emplace(backend.createBuffer(sizeof(InstanceBufferObject) * 2, VulkanBufferUsageType::Vertex));
    }
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
    std::string file = "Textures/chalet_texture.texture";
    ASSIGN_OR_RETURN(Core::IO::InputStream input, Core::IO::OpenFileForRead(file));

    auto textureAsset = input.read<Assets::Texture>();

    VulkanLogicalDevice& device = backend.getLogicalDevice();
    texture                     = backend.createTexture(
          Core::ToSpan(textureAsset.data), VK_FORMAT_R8G8B8A8_UNORM, {textureAsset.width, textureAsset.height});

    return Core::Status::Ok();
}

Core::Status initVulkanBackend(VulkanRendererBackend& backend) {
    RETURN_IF_ERROR(createVertexBuffer(backend));
    RETURN_IF_ERROR(createTextureImage(backend));

    createCommandBuffers(backend);

    imageAvailableSemaphores.ensureCapacity(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.ensureCapacity(MAX_FRAME_IN_FLIGHT);
    queueFences.ensureCapacity(MAX_FRAME_IN_FLIGHT);
    for(int i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
        imageAvailableSemaphores.emplace(VulkanSemaphore::Create(backend.getLogicalDevice()));
        renderFinishedSemaphores.emplace(VulkanSemaphore::Create(backend.getLogicalDevice()));
        queueFences.emplace(VulkanFence::Create(backend.getLogicalDevice(), VulkanFenceState::Signaled));
    }

    vkQueueWaitIdle(backend.getQueues().transfer);
    backend.processFinishedSubmitResources();

    return Core::Status::Ok();
}

void recreateSwapChain(GUI::Window& window, VulkanRendererBackend& backend) {
    // Handle minimization (extent becomes 0)
    int width = 0, height = 0;
    glfwGetFramebufferSize(window.getHandle(), &width, &height);
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(window.getHandle(), &width, &height);
        glfwWaitEvents();
    }

    backend.remakeSwapChain();
}

VkCommandBuffer RenderGUI(uint32_t framebufferIndex, VulkanRendererBackend& backend) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool windowShowned = true;

    ImGui::ShowDemoWindow(&windowShowned);
    VkCommandBuffer MainBuffer = mainDrawBuffers[framebufferIndex];
    VK_CHECK(vkResetCommandBuffer(MainBuffer, 0));

    VkCommandBuffer GuiCommandBuffer = guiDrawBuffers[framebufferIndex];
    VK_CHECK(vkResetCommandBuffer(GuiCommandBuffer, 0));

    VulkanSwapChain& swapChain = *backend.getSwapChain();

    VkCommandBufferInheritanceInfo inheritance = {};
    inheritance.sType                          = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance.renderPass                     = *backend.getSwapChainRenderPass();
    inheritance.subpass                        = 0;
    inheritance.framebuffer                    = swapChain.framebuffers[framebufferIndex];

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    beginInfo.pInheritanceInfo         = &inheritance;    // Optional

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
    VulkanSwapChain& swapChain  = *backend.getSwapChain();

    static UniformBufferObject ubo = {
          glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1)),
          glm::perspective(glm::radians(45.0f),
                           swapChain.viewport.width / static_cast<float>(swapChain.viewport.height),
                           0.1f,
                           10000.0f)};

    static Core::Array<InstanceBufferObject> instances = {
          InstanceBufferObject{glm::mat4(1.0f)},
          InstanceBufferObject{glm::mat4(1.0f)},
    };

    for(uint32_t i = 0; i < instances.count(); i++) {
        float rads  = glm::radians(90.0f);
        float angle = delta.count() * rads * (i + 1);

        instances[i].model = glm::rotate(instances[i].model, angle, glm::vec3(0, 0, 1));
    }


    if(window.hasResized(true)) {
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

    ASSERT_WITH_MESSAGE(queueFences[currentFrame].waitAndReset(device), "Failed to wait for queue fence!");

    uniformBuffers[currentFrame].upload(Core::ToBytes(ubo));
    instanceBuffers[currentFrame].upload(Core::AsBytes(instances));

    recordCommandBuffers(currentFrame, backend);
    VkCommandBuffer buffer = RenderGUI(currentFrame, backend);

    queues.graphics.submit(buffer,
                           VulkanQueueSubmitType::Graphics,
                           imageAvailableSemaphores[currentFrame],
                           renderFinishedSemaphores[currentFrame],
                           queueFences[currentFrame]);

    VkResult presentResult = queues.present.present(swapChain, renderFinishedSemaphores[currentFrame], currentFrame);

    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;

    if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        Core::Log::Info(Test, "Not recreating swap chain");
        recreateSwapChain(window, backend);
    } else if(presentResult != VK_SUCCESS) {
        Core::Log::Error(Test, "Failed to present image: {}", presentResult);
    }
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

    RETURN_IF_ERROR(initVulkanBackend(backend));

    VulkanSwapChain& swapChain = *backend.getSwapChain();
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

    std::thread drawThread([&]() {
        while(!window->shouldClose()) {
            frameCount++;
            ticker.tick();
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
    });
    while(!window->shouldClose()) {
        glfwPollEvents();
    }
    drawThread.join();

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