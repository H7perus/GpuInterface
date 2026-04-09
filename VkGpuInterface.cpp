#pragma once
#include "VkGpuInterface.h"
#include "SDL3/SDL_vulkan.h"
#include "Types/Swapchain.h"
#include "VkHelpers.h"

#include "Types/Buffer.h"
#include "Types/DescriptorHeapBuffer.h"
#include "Types/DeviceManager.h"
#include "Types/PipelineCompute.h"
#include "Types/PipelineGraphics.h"
#include "Types/Sampler.h"
#include "Types/Texture.h"
#include "glm/glm.hpp"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_include.h"


VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;


#include "ShaderCompile/ShaderCompile.h"
#include "ShaderCompile/SlangCompileContext.h"
#include "ShaderCompile/SlangCompiledUnit.h"

#include "ShaderCompile/GlslShaderCompile.h"

#include <string>


f32 vertexInfo[] = {0.0, -1.0, 0.5, 1.0, 0.0, 0.0, 0.5, 0.0, -1.0, 1.0, 0.5, 0.0,
                    1.0, 0.0,  0.0, 1.0, 1.0, 1.0, 0.5, 0.0, 0.0,  1.0, 1.0, 1.0};

int vertexIndices[] = {0, 1, 2};


void KE::VkGpuInterface::Init()
{
    vkboot_inst_ = createInstance();
    vk_inst_     = vkboot_inst_.instance;




    VK::ContextManager::Init();

    VK::ContextManager::SetVkInstance(vkboot_inst_);

    vk::SurfaceKHR surface;
    bool           test = SDL_Vulkan_CreateSurface(window_, vk_inst_, nullptr, (VkSurfaceKHR*)&surface);

    VK::ContextManager::GetInstance().AddDevice(createDevice(vkboot_inst_, surface));

    VK::Device& device = VK::ContextManager::GetInstance().GetDevice(0);


    VK::ContextManager::AddSwapchain(0, surface, 800, 600, 2);

    // swapchain_.Init(0, surface, 800, 600);

    graphics_queue_ = device.GetGraphicsQueue();
    compute_queue_  = device.GetComputeQueue();

    computeCmdPool = device.CreateComputeCommandPool();


    KE::VK::SlangCompileContext compileContext;

    SlangCompiledUnit slangShader =
        compileContext.CompileShaderPath("../../../../../../Engine/src/GpuInterface/shaders/slangcompute.slang");

    SlangCompiledUnit slangGraphicsShader =
        compileContext.CompileShaderPath("../../../../../../Engine/src/GpuInterface/shaders/combinedVertFrag.slang");


    Slang::ComPtr<slang::IBlob> blob = slangGraphicsShader.getTargetCode();

    auto glslComputeSpv =
        CompileGlslShader("../../../../../../Engine/src/GpuInterface/shaders/glslcompute.comp", shaderc_compute_shader);
    std::ofstream file("spvDescriptorHeapTEST2.spv", std::ios::binary);
    file.write((char*)blob.get()->getBufferPointer(), blob.get()->getBufferSize());
    file.close();


    auto vertTest =
        CompileGlslShader("../../../../../../Engine/src/GpuInterface/shaders/glsl.vert", shaderc_vertex_shader);
    auto fragTest =
        CompileGlslShader("../../../../../../Engine/src/GpuInterface/shaders/glsl.frag", shaderc_fragment_shader);

    KE::VK::PipelineCompute pipeline(0, slangShader);

    testPipeline = KE::VK::PipelineGraphics(0, slangGraphicsShader);
    // testPipeline = KE::VK::PipelineGraphics(0, vertTest, fragTest);

    vertexBuffer = KE::VK::Buffer(0, sizeof(vertexInfo), vk::BufferUsageFlagBits::eVertexBuffer,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    f32* mappedVertBuffer = (float*)vertexBuffer.map();
    memcpy(mappedVertBuffer, vertexInfo, sizeof(vertexInfo));

    for (int i = 0; i < 10; i++)
        std::cout << "TEST VALUE: " << std::dec << mappedVertBuffer[i] << std::endl;

    vertexBuffer.unmap();


    

    testSampler = VK::Sampler(0, true);

    u8* textureData = new u8[1024 * 1024 * 4];

    int            w;
    int            h;
    int            comp;
    unsigned char* image = stbi_load("E:/Textures/cat pics/cat_cigarette.jpg", &w, &h, &comp, STBI_rgb_alpha);

    testTexture = KE::VK::Texture(0, w, h, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled,
                                  vk::MemoryPropertyFlagBits::eDeviceLocal, true);

    if (!image)
    {
    }

    std::string error = stbi_failure_reason();

    for (int x = 0; x < 1024; x++)
        for (int y = 0; y < 1024; y++)
            for (int c = 0; c < 4; c++)
            {
                textureData[(x + y * 1024) * 4 + c] = (((x / 128) + (y / 128)) % 2 * 255) * (c != 1);
            }

    testTexture.UploadPixels(image);

    testBuff = VK::Buffer(0, 1024, vk::BufferUsageFlagBits::eStorageBuffer,
                          vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, true);

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;


    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool        = computeCmdPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer = vk::Device(device).allocateCommandBuffers(allocInfo)[0];

    commandBuffer.begin(beginInfo);

    glm::uvec2 pushIndex = glm::uvec2(6048, 6048);

    vk::PushDataInfoEXT pushInfo{};
    pushInfo.offset       = 0;
    pushInfo.data.address = &pushIndex;
    pushInfo.data.size    = sizeof(pushIndex);


    commandBuffer.pushDataEXT(&pushInfo);

    commandBuffer.bindResourceHeapEXT(device.GetResourceHeapBindInfoPtr());


    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

    commandBuffer.dispatch(4, 1, 1);

    // End recording
    commandBuffer.end();

    // Submit to compute queue
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    compute_queue_.submit(1, &submitInfo, nullptr); // no fence for now
    compute_queue_.waitIdle();                      // wait for completion (blocking)

    u32* numberPointer = (u32*)(testBuff.map());


    for (int i = 0; i < 1024 / 4; i++)
        std::cout << numberPointer[i] << std::endl;
}


u64 KE::VkGpuInterface::GetSDLWindowFlag()
{
    return SDL_WINDOW_VULKAN;
}