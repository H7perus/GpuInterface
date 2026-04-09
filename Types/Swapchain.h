#pragma once

#include "Device.h"
#include "Frame.h"
#include "SwapchainOutput.h"

#include "vulkan/vulkan.hpp"
#include "VkBootstrap.h"

#include <vector>

#include "GpuInterface/GpuInterface.h"

#include "GpuInterface/BasicTypeAliases.h"

namespace KE::VK
{
    class GPUI_DLL_API Swapchain
    {
      public:
        vk::SwapchainKHR swapchain_;
        vk::Format       format_;
        vk::Extent2D     extent_;
        vk::SurfaceKHR   surface_;
        vkb::Swapchain   vkbSwapchain_;
        
        vk::Queue        graphicsQueue_;
        // refers to Frame In Flight!
        u32                        currentFrame_;
        u32                        currentImage_;
        u32 deviceIndex_;
        std::vector<InflightFrame>         frames_;
        std::vector<vk::Image>     images_;
        std::vector<vk::ImageView> imageViews_;

        ResourceSet swapchainOutputSet_;


        Swapchain() = default;

        Swapchain(u32 deviceIndex, vk::SurfaceKHR surface, uint32_t width, uint32_t height, u32 framesInFlight)
        {
            Init(deviceIndex, surface, width, height, framesInFlight);
        }

        void Init(u32 deviceIndex, vk::SurfaceKHR surface, uint32_t width, uint32_t height, u32 framesInFlight);

        InflightFrame &BeginNextFrame();

        vk::Image GetCurrentImage()
        {
            return images_[currentImage_];
        }
        vk::ImageView GetCurrentImageView()
        {
            return imageViews_[currentImage_];
        }

        vk::CommandBuffer GetCurrentCommandBuffer()
        {
            return frames_[currentFrame_].commandBuffer_;
        }

        ResourceHandle GetSwapchainOutputHandle()
        {
            return swapchainOutputSet_.GetResourceHandle();
        }

        void EndFrame();

        void BuildSwapchain(u32 width, u32 height);

        void Recreate(u32 width, u32 height);

        void Destroy();
    };
}