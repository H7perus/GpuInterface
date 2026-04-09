#pragma once 

#include "Resource.h"
#include "Device.h"

#include "vulkan/vulkan.hpp"
#include "GpuInterfaceDLL.h"

namespace KE::VK
{
    //This class represents the Vulkan objects that belong to one frame-in-flight. The number is equal to the swapchain images
    class GPUI_DLL_API InflightFrame : public IResource
    {
        friend class Swapchain;
        KE_GPUREFLECT(InflightFrame)
    public:
      void Init(KE::VK::Device &device);
      void Destroy(KE::VK::Device &device);
      vk::SubmitInfo *GetSubmitInfoPtr();
    private:
        //For fence waits etc.
        KE::VK::Device* device_;
        //Command pool, required per frame, because we want to reset the pool after submission
        vk::CommandPool commandPool_;
        //Self explanatory. We could also write into multiple buffers, might become necessary for certain applications
        vk::CommandBuffer commandBuffer_;
        //So the GPU won't write into the framebuffer before it is free. We don't want to write into the front-buffer
        vk::Semaphore imageAvailableSemaphore_;
        //Signaling to the GPU when flipping is allowed, i.e. when we are done with the frame
        vk::Semaphore renderFinishedSemaphore_;
        //Signaling the CPU when it can start working. If the previous frame is not truely finished, we might overwrite UBOs etc. before it is ok to do so
        vk::Fence inFlightFence_;
        //Saving the submit info for the frame, so we don't have to create it every time
        vk::SubmitInfo submitInfo_;
        //Having this all together means we can just pass the Frame as a resource.
        vk::ImageLayout currentLayout_;
    };
}