// Test.cpp : This file contains the 'main' function. Program execution begins
// and ends there.
//
#include "core/logging/Logger.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "core/io/file_system/FileSystem.h"
#include "core/io/file_system/Path.h"
#include "core/io/file_system/VirtualFileSystemMount.h"
#include "core/time/SystemClockTicker.h"
#include "core/time/Timer.h"

#include "gui/Window.h"

#include "renderer/backends/vulkan/DiagnosticCheckpoint.h"
#include "renderer/backends/vulkan/VulkanRendererBackend.h"

#include "renderer/backends/vulkan/VulkanBuffer.h"
#include "renderer/backends/vulkan/VulkanDescriptorPool.h"
#include "renderer/backends/vulkan/VulkanGraphicsPipeline.h"

#include "renderer/resources/GPUMesh.h"
#include "renderer/resources/GPUTexture.h"

#include "assets/models/Mesh.h"
#include "assets/textures/Texture.h"

#include <imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>


Core::LogCategory Test("Test");
using namespace Renderer::Backends::Vulkan;

VulkanDescriptorPool descriptorPool;
VulkanGraphicsPipeline graphicsPipeline;
Core::Array<VkDescriptorSet> uniformBuffersDescriptors;
Core::Array<VulkanBuffer> uniformBuffers;
Core::Array<VulkanBuffer> instanceBuffers;

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

void createDrawDataBuffers(VulkanRendererBackend& backend) {
    VulkanLogicalDevice& device = backend.getLogicalDevice();
    VulkanSwapChain& swapChain  = *backend.getSwapChain();

    uint32_t swapChainCount = swapChain.framebuffers.size();

    descriptorPool = VulkanDescriptorPool::Create(
          device, swapChainCount + 1, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
    uniformBuffersDescriptors =
          descriptorPool.allocateSets(device, swapChainCount, graphicsPipeline.descriptorSetLayout);

    instanceBuffers.ensureCapacity(swapChainCount);
    uniformBuffers.ensureCapacity(swapChainCount);

    for(uint64_t i = 0; i < swapChainCount; i++) {
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

    texture = backend.createTexture(
          Core::ToSpan(textureAsset.data), VK_FORMAT_R8G8B8A8_UNORM, {textureAsset.width, textureAsset.height});

    return Core::Status::Ok();
}

Core::Status initVulkanBackend(VulkanRendererBackend& backend) {
    RETURN_IF_ERROR(createVertexBuffer(backend));
    RETURN_IF_ERROR(createTextureImage(backend));

    createDrawDataBuffers(backend);

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

void RenderGUI() {
    static bool windowShowned = true;
    ImGui::ShowDemoWindow(&windowShowned);
}

Core::Status drawFrame(GUI::Window& window, Core::Clock::Seconds delta, VulkanRendererBackend& backend) {
    VulkanSwapChain& swapChain = *backend.getSwapChain();

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

    instanceBuffers[currentFrame].upload(Core::AsBytes(instances));
    uniformBuffers[currentFrame].upload(Core::ToBytes(ubo));

    Core::Array<VulkanFrameCommands> commandList;
    VulkanFrameCommands& commands = commandList.emplace();

    VulkanDrawMeshInstancedCommand& draw = commands.instanceMeshCommands.emplace();
    draw.mesh                            = &mesh;
    draw.pipeline                        = graphicsPipeline;
    draw.uniformDescriptorSet            = uniformBuffersDescriptors[currentFrame];
    draw.instanceDataBuffer              = instanceBuffers[currentFrame];
    draw.instanceCount                   = 2;

    commands.customCommands.emplace([](VkCommandBuffer commandBuffer) {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    });

    return backend.submitCommands(commandList);
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

    while(!window->shouldClose()) {
        glfwPollEvents();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        frameCount++;
        ticker.tick();

        RenderGUI();

        Core::Status drawStatus = drawFrame(*window, clock.tickedTime(), backend);
        if(drawStatus.isError()) {
            Core::Log::Error(Test, drawStatus.message());
        }

        // clock.timeScale += changePerSecond * clock2.tickedSeconds().count();

        if(timer.elapsedTime() > period) {
            changePerSecond *= -1;
            timer.reset();
        }

        if(frameCount == 1000) {
            Core::Clock::Seconds now = clock.totalElapsedSeconds();
            auto duration            = now - lastFPSCalculation;

            Core::Log::Info(Test, "{}ms ({} fps)", duration.count() / frameCount, frameCount / duration.count());
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